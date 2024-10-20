use std::fs::File; // Importa la estructura File para manejar archivos.  
use std::io::{self, Read}; // Importa módulos de entrada/salida para leer archivos.  
use std::path::Path; // Importa Path para manejar rutas de archivos.  
use std::process::Command; // Importa Command para ejecutar comandos del sistema.  
use std::thread; // Importa el módulo de hilos para manejar la ejecución concurrente.  
use std::time::Duration; // Importa Duration para manejar tiempos de espera.  
use serde::{Deserialize, Serialize}; // Importa Serde para serializar y deserializar datos.  
use reqwest::blocking::Client; // Importa el cliente HTTP de Reqwest para hacer peticiones.  
use chrono::Local; // Importa Chrono para manejar fechas y horas.  
use ctrlc; // Importa CtrlC para capturar señales de interrupción como Ctrl+C.  

// Define una estructura para almacenar información del sistema.  
#[derive(Debug, Serialize, Deserialize)]  
struct SystemInfo {  
    processes: Vec<Process>, // Define un vector de procesos dentro de SystemInfo.  
}  

// Define una estructura para almacenar información de cada proceso.  
#[derive(Debug, Serialize, Deserialize, PartialEq)]  
struct Process {  
    pid: u32, // Identificador del proceso.  
    name: String, // Nombre del proceso.  
    cmd_line: String, // Línea de comandos usada para ejecutar el proceso.  
    memory_usage: f64, // Uso de memoria del proceso.  
    cpu_usage: f64, // Uso de CPU del proceso.  
    vsz: u64, // Tamaño virtual del proceso.  
    rss: u64, // Tamaño de la memoria residente del proceso.  
}  

// Implementa el trait Eq para permitir comparaciones de igualdad.  
impl Eq for Process {}  

// Implementa el trait Ord para permitir ordenamientos completos.  
impl Ord for Process {  
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {  
        // Compara primero por uso de CPU, luego por memoria, VSZ y RSS.  
        self.cpu_usage.partial_cmp(&other.cpu_usage).unwrap_or(std::cmp::Ordering::Equal)  
            .then_with(|| self.memory_usage.partial_cmp(&other.memory_usage).unwrap_or(std::cmp::Ordering::Equal))  
            .then_with(|| self.vsz.cmp(&other.vsz))  
            .then_with(|| self.rss.cmp(&other.rss))  
    }  
}  

// Implementa el trait PartialOrd para permitir comparaciones parciales.  
impl PartialOrd for Process {  
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {  
        Some(self.cmp(other)) // Usa la comparación completa definida en Ord.  
    }  
}  

// Estructura para almacenar información de logs de procesos.  
#[derive(Clone)]  
struct LogProcess {  
    pid: u32, // Identificador del proceso.  
    container_id: String, // ID del contenedor.  
    name: String, // Nombre del proceso.  
    memory_usage: f64, // Uso de memoria del proceso.  
    cpu_usage: f64, // Uso de CPU del proceso.  
}  

fn main() {  
    // Configura un manejador para capturar Ctrl+C y finalizar el servicio.  
    ctrlc::set_handler(move || {  
        println!("Finalizando el servicio...");  
        finalizar_servicio(); // Llama a la función para finalizar el servicio.  
        std::process::exit(0); // Termina el programa.  
    }).expect("Error en la instalación de Ctrl+C");  

    // Inicia el contenedor de logs y guarda su ID.  
    let log_container_id = iniciar_contenedor_de_logs();  
    let mut log_proc_list: Vec<LogProcess> = Vec::new(); // Vector para almacenar logs de procesos.  

    loop {  
        // Aquí se manda a leer el archivo.  
        if let Ok(system_info) = leer_y_deserializar_archivo("/proc/sysinfo_201700747") {  
            // Si la lectura es exitosa, se analiza y gestiona la información del sistema.  
            analizar_y_gestionar(system_info, &log_container_id, &mut log_proc_list);  
        } else {  
            eprintln!("Error al leer o deserializar el archivo."); // Imprime un error si falla.  
        }  

        thread::sleep(Duration::from_secs(10)); // Espera 10 segundos antes de la siguiente iteración.  
    }  
}  

// Función para iniciar el contenedor de logs.  
fn iniciar_contenedor_de_logs() -> String {  
    // Ejecuta un comando Docker para iniciar el contenedor de logs.  
    let output = Command::new("docker")  
        .args(&["run", "-d", "--name", "log_container", "log_image"])  
        .output()  
        .expect("Failed to start log container");  

    // Convierte la salida del comando a String y la devuelve.  
    let container_id = String::from_utf8_lossy(&output.stdout).trim().to_string();  
    println!("Contenedor de logs iniciado con ID: {}", container_id);  
    container_id  
}  

// Función para leer y deserializar el archivo de información del sistema.  
fn leer_y_deserializar_archivo(ruta: &str) -> Result<SystemInfo, io::Error> {  
    let path = Path::new(ruta); // Crea un objeto Path con la ruta proporcionada.  
    let mut file = File::open(path)?; // Abre el archivo en modo lectura.  
    let mut content = String::new(); // Crea un String vacío para almacenar el contenido.  
    file.read_to_string(&mut content)?; // Lee el contenido del archivo en el String.  
    let system_info: SystemInfo = serde_json::from_str(&content)?; // Deserializa el JSON a SystemInfo.  
    Ok(system_info) // Devuelve el SystemInfo deserializado.  
}  

// Función para analizar y gestionar los procesos.  
fn analizar_y_gestionar(system_info: SystemInfo, log_container_id: &str, log_proc_list: &mut Vec<LogProcess>) {  
    let mut processes = system_info.processes; // Obtiene la lista de procesos.  
    processes.sort(); // Ordena los procesos según los criterios definidos.  

    // Divide la lista de procesos en dos mitades.  
    let (lowest_list, highest_list) = processes.split_at(processes.len() / 2);  

    // Imprime los procesos de bajo consumo.  
    println!("Bajo consumo:");  
    for process in lowest_list.iter().take(3) {  
        println!("{:?}", process);  
    }  

    // Imprime los procesos de alto consumo.  
    println!("Alto consumo:");  
    for process in highest_list.iter().rev().take(2) {  
        println!("{:?}", process);  
    }  

    let client = Client::new(); // Crea un nuevo cliente HTTP.  
    let timestamp = Local::now(); // Obtiene la fecha y hora actual.  

    // Itera sobre los procesos de bajo consumo, omitiendo los primeros 3.  
    for process in lowest_list.iter().skip(3) {  
        if process.cmd_line.contains(log_container_id) {  
            continue; // No detiene el contenedor de logs.  
        }  
        detener_contenedor(&process.cmd_line); // Detiene el contenedor.  
        enviar_log(&client, process, timestamp); // Envía un log del proceso.  

        // Agrega el proceso al log.  
        log_proc_list.push(LogProcess {  
            pid: process.pid,  
            container_id: process.cmd_line.clone(),  
            name: process.name.clone(),  
            memory_usage: process.memory_usage,  
            cpu_usage: process.cpu_usage,  
        });  
    }  

    // Itera sobre los procesos de alto consumo, omitiendo los últimos 2.  
    for process in highest_list.iter().take(highest_list.len() - 2) {  
        if process.cmd_line.contains(log_container_id) {  
            continue; // No detiene el contenedor de logs.  
        }  
        detener_contenedor(&process.cmd_line); // Detiene el contenedor.  
        enviar_log(&client, process, timestamp); // Envía un log del proceso.  

        // Agrega el proceso al log.  
        log_proc_list.push(LogProcess {  
            pid: process.pid,  
            container_id: process.cmd_line.clone(),  
            name: process.name.clone(),  
            memory_usage: process.memory_usage,  
            cpu_usage: process.cpu_usage,  
        });  
    }  

    // Imprime los logs de los procesos detenidos.  
    println!("Contenedores matados:");  
    for log in log_proc_list.iter() {  
        println!(  
            "PID: {}, Name: {}, Container ID: {}, Memory Usage: {}, CPU Usage: {}",  
            log.pid, log.name, log.container_id, log.memory_usage, log.cpu_usage  
        );  
    }  
    println!("------------------------------");  
}  

// Función para detener un contenedor.  
fn detener_contenedor(container_id: &str) {  
    let _output = Command::new("docker")  
        .args(&["stop", container_id])  
        .output()  
        .expect("Failed to stop container");  
    println!("Contenedor detenido: {}", container_id);  
}  

// Función para enviar un log de un proceso.  
fn enviar_log(client: &Client, process: &Process, timestamp: chrono::DateTime<Local>) {  
    let log_entry = format!(  
        "Timestamp: {}, PID: {}, Name: {}, Memory Usage: {}, CPU Usage: {}",  
        timestamp, process.pid, process.name, process.memory_usage, process.cpu_usage  
    );  

    let _res = client.post("http://log_container/logs")  
        .body(log_entry)  
        .send();  

    println!("Log enviado para el proceso: {}", process.pid);  
}  

// Función para finalizar el servicio.  
fn finalizar_servicio() {  
    let client = Client::new(); // Crea un nuevo cliente HTTP.  
    let _res = client.post("http://log_container/finalize")  
        .send();  
    println!("Servicio finalizado y cronjob eliminado.");  
}