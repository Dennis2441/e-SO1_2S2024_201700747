#!/bin/bash  

# Construir las imágenes  
docker build -t high-consumption ./high  
docker build -t low-consumption ./low  

# Función para generar un nombre aleatorio usando /dev/urandom  
generate_random_name() {  
    echo "container_$(cat /dev/urandom | tr -dc 'a-z0-9' | fold -w 8 | head -n 1)"  
}  

# Crear 10 contenedores aleatoriamente  
for i in {1..10}; do  
    if (( RANDOM % 2 == 0 )); then  
        # Alto consumo  
        container_name=$(generate_random_name)  
        docker run -d --name "$container_name" high-consumption  
    else  
        # Bajo consumo  
        container_name=$(generate_random_name)  
        docker run -d --name "$container_name" low-consumption  
    fi  
done