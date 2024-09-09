import time  
import math  

# Crear una lista grande para consumir RAM  
large_list = [i for i in range(10000000)]  # Ajusta el tamaño según sea necesario  

while True:  
    # Operaciones intensivas de CPU  
    _ = [math.sqrt(i) for i in range(1000)]  
    
    # Simular un uso de RAM  
    _ = [x * 2 for x in large_list]  
    
    time.sleep(0.1)