#!/bin/bash

# Número de contenedores a crear
num_containers=10

# Nombre base de los contenedores
container_base_name="container"

# Crear contenedores
for i in $(seq 1 $num_containers); do
  container_name="${container_base_name}_${i}"
  docker run -d --name $container_name alpine
  echo "Contenedor $container_name creado."
done