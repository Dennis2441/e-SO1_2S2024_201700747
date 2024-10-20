#include <linux/module.h>  // Necesario para el funcionamiento de módulos  
#include <linux/kernel.h>  // Necesario para definiciones del núcleo  
#include <linux/string.h>  // Para manipulación de cadenas  
#include <linux/init.h>    // Para inicialización de módulos  
#include <linux/proc_fs.h> // Para crear entradas en el sistema de archivos /proc  
#include <linux/seq_file.h> // Para manejo de archivos secuenciales  
#include <linux/mm.h>      // Para estructuras de gestión de memoria  
#include <linux/sched.h>   // Para estructuras de programación de procesos  
#include <linux/timer.h>   // Para temporizadores  
#include <linux/jiffies.h> // Para manejo de tiempos (jiffies)  
#include <linux/uaccess.h> // Para acceso a la memoria de usuario  
#include <linux/tty.h>     // Para acceso a terminal  
#include <linux/sched/signal.h> // Para señales de procesos  
#include <linux/fs.h>      // Para manejo de sistemas de archivos  
#include <linux/slab.h>    // Para gestión de memoria dinámica  
#include <linux/sched/mm.h> // Para estructuras de memoria de procesos  
#include <linux/binfmts.h> // Para formatos de binarios  
#include <linux/timekeeping.h> // Para manejar tiempos  

MODULE_LICENSE("GPL"); // Licencia del módulo  
MODULE_AUTHOR("Tu Nombre"); // Nombre del autor  
MODULE_DESCRIPTION("Modulo para leer información de memoria y CPU en JSON"); // Descripción del módulo  
MODULE_VERSION("1.0"); // Versión del módulo  

#define PROC_NAME "sysinfo_201700747" // Nombre del archivo creado en /proc  
#define MAX_CMDLINE_LENGTH 256 // Longitud máxima para la línea de comandos  
#define CONTAINER_ID_LENGTH 64 // Longitud máxima para el ID del contenedor  

// Función para obtener la línea de comandos de un proceso y retornar un puntero a la cadena  
static char *get_process_cmdline(struct task_struct *task) {  
    /*   
        Creamos una estructura mm_struct para obtener la información de memoria  
        Creamos un apuntador char para la línea de comandos  
        Creamos un apuntador char para recorrer la línea de comandos  
        Creamos variables para guardar las direcciones de inicio y fin de los argumentos y el entorno  
        Creamos variables para recorrer la línea de comandos  
    */  
    struct mm_struct *mm; // Estructura para la memoria del proceso  
    char *cmdline, *p; // Puntero para la línea de comandos y uno para recorrerla  
    unsigned long arg_start, arg_end, env_start; // Variables para las direcciones de inicio y fin  
    int i, len; // Variables para longitud y bucles  

    // Reservamos memoria para la línea de comandos  
    cmdline = kmalloc(MAX_CMDLINE_LENGTH, GFP_KERNEL);  
    if (!cmdline)  
        return NULL; // Retorna NULL si no se puede reservar memoria  

    // Obtenemos la información de memoria del proceso  
    mm = get_task_mm(task);  
    if (!mm) {  
        kfree(cmdline); // Libre la memoria si no hay estructura mm válida  
        return NULL;  
    }  

    /*   
       1. Primero obtenemos el bloqueo de lectura de la estructura mm_struct para una lectura segura  
       2. Obtenemos las direcciones de inicio y fin de los argumentos y el entorno  
       3. Liberamos el bloqueo de lectura de la estructura mm_struct  
    */  
    down_read(&mm->mmap_lock); // Bloqueo de lectura  
    arg_start = mm->arg_start; // Dirección de inicio de argumentos  
    arg_end = mm->arg_end; // Dirección de fin de argumentos  
    env_start = mm->env_start; // Dirección de inicio del entorno  
    up_read(&mm->mmap_lock); // Desbloqueo de lectura  

    // Calculamos la longitud de la línea de comandos y validamos que no sea mayor a MAX_CMDLINE_LENGTH - 1  
    len = arg_end - arg_start;  
    if (len > MAX_CMDLINE_LENGTH - 1)  
        len = MAX_CMDLINE_LENGTH - 1; // Ajustamos la longitud si es necesario  

    // Obtenemos la línea de comandos de la memoria virtual del proceso  
    if (access_process_vm(task, arg_start, cmdline, len, 0) != len) {  
        mmput(mm); // Libera la estructura mm  
        kfree(cmdline); // Libera memoria de la línea de comandos  
        return NULL; // Retorna NULL si ocurre un error  
    }  

    // Agregamos un carácter nulo al final de la línea de comandos  
    cmdline[len] = '\0';  

    // Reemplazamos caracteres nulos por espacios  
    p = cmdline; // Pointer a la línea de comandos  
    for (i = 0; i < len; i++)  
        if (p[i] == '\0')  
            p[i] = ' '; // Reemplazamos '\0' con ' '  

    // Liberamos la estructura mm_struct  
    mmput(mm); // Liberamos la estructura de memoria del proceso  
    return cmdline; // Retornamos la línea de comandos  
}  

/*   
    Función para mostrar la información en el archivo /proc en formato JSON  
*/  
static int sysinfo_show(struct seq_file *m, void *v) {  
    /*   
        Creamos una estructura sysinfo para obtener la información de memoria  
        Creamos una estructura task_struct para recorrer los procesos  
        total_jiffies para obtener el tiempo total de CPU  
        first_process para saber si es el primer proceso que se está imprimiendo  
    */  
    struct sysinfo si; // Estructura para información del sistema  
    struct task_struct *task; // Estructura para iterar sobre los procesos  
    unsigned long total_jiffies = jiffies; // Total de jiffies al momento de la consulta  
    int first_process = 1; // Indicador para el primer proceso  

    // Obtenemos la información de memoria  
    si_meminfo(&si); // Rellenamos la estructura si  

    // Iniciamos el formato JSON para la salida  
    seq_printf(m, "{\n");  
    seq_printf(m, "\"Processes\": [\n");  

    // Iteramos sobre todos los procesos  
    for_each_process(task) {  
        if (strcmp(task->comm, "containerd-shim") == 0) { // Verificamos si el proceso es de tipo contenedor  
            unsigned long vsz = 0; // Variable para el tamaño virtual del proceso  
            unsigned long rss = 0; // Variable para RSS (Resident Set Size)  
            unsigned long totalram = si.totalram * 4; // Total de RAM en sistema  
            unsigned long mem_usage = 0; // Variable para uso de memoria  
            unsigned long cpu_usage = 0; // Variable para uso de CPU  
            char *cmdline = NULL; // Puntero a la línea de comandos  

            // Obtenemos los valores de VSZ y RSS  
            if (task->mm) { // Verificamos si el proceso tiene una estructura de memoria válida  
                vsz = task->mm->total_vm << (PAGE_SHIFT - 10); // Calculamos el tamaño virtual  
                rss = get_mm_rss(task->mm) << (PAGE_SHIFT - 10); // Calculamos el tamaño residente  
                mem_usage = (rss * 10000) / totalram; // Porcentaje de uso de memoria  
            }  

            /*   
                Obtenemos el tiempo total de CPU de un proceso  
                Obtenemos el tiempo total de CPU de todos los procesos  
                Obtenemos el uso de CPU en porcentaje  
                Obtenemos la línea de comandos de un proceso  
            */  
            unsigned long total_time = task->utime + task->stime; // Tiempo total de CPU del proceso  
            cpu_usage = (total_time * 10000) / total_jiffies; // Porcentaje de uso de CPU  
            cmdline = get_process_cmdline(task); // Obtiene la línea de comandos del proceso  

            // Manejo de formato para la salida JSON  
            if (!first_process) {  
                seq_printf(m, ",\n"); // Añade una coma si no es el primer proceso  
            } else {  
                first_process = 0; // Si es el primer proceso, cambia el indicador  
            }  

            // Imprime la información del proceso  
            seq_printf(m, "  {\n");  
            seq_printf(m, "    \"PID\": %d,\n", task->pid); // Imprime el PID  
            seq_printf(m, "    \"Name\": \"%s\",\n", task->comm); // Imprime el nombre del proceso  
            seq_printf(m, "    \"Cmdline\": \"%s\",\n", cmdline ? cmdline : "N/A"); // Imprime la línea de comandos  
            seq_printf(m, "    \"MemoryUsage\": %lu.%02lu,\n", mem_usage / 100, mem_usage % 100); // Imprime el uso de memoria  
            seq_printf(m, "    \"CPUUsage\": %lu.%02lu\n", cpu_usage / 100, cpu_usage % 100); // Imprime el uso de CPU  
            seq_printf(m, "  }");  

            // Liberamos la memoria de la línea de comandos  
            if (cmdline) {  
                kfree(cmdline); // Libera la memoria reservada para cmdline  
            }  
        }  
    }  

    // Cierra la estructura JSON  
    seq_printf(m, "\n]\n}\n");  
    return 0; // Retorna 0 indicando éxito  
}  

/*   
    Función que se ejecuta al abrir el archivo /proc  
*/  
static int sysinfo_open(struct inode *inode, struct file *file) {  
    return single_open(file, sysinfo_show, NULL); // Abre el archivo y llama a sysinfo_show  
}  

/*   
    Estructura que contiene las operaciones del archivo /proc  
*/  
static const struct proc_ops sysinfo_ops = {  
    .proc_open = sysinfo_open, // Función para abrir el archivo  
    .proc_read = seq_read, // Función para leer del archivo  
};  

/*   
    Función de inicialización del módulo  
*/  
static int __init sysinfo_init(void) {  
    proc_create(PROC_NAME, 0, NULL, &sysinfo_ops); // Crea el archivo en /proc  
    printk(KERN_INFO "sysinfo_json módulo cargado\n"); // Mensaje en el kernel  
    return 0; // Retorna 0 indicando éxito  
}  

/*   
    Función de limpieza del módulo  
*/  
static void __exit sysinfo_exit(void) {  
    remove_proc_entry(PROC_NAME, NULL); // Elimina el archivo de /proc  
    printk(KERN_INFO "sysinfo_json módulo desinstalado\n"); // Mensaje en el kernel  
}  

module_init(sysinfo_init); // Macro para indicar la función de inicialización  
module_exit(sysinfo_exit); // Macro para indicar la función de limpieza