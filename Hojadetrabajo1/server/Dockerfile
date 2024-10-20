# Usar una imagen base de Go
FROM golang:1.17-alpine

# Establecer el directorio de trabajo dentro del contenedor
WORKDIR /app

# Copiar el contenido de la app al contenedor
COPY . .

# Construir la aplicación
RUN go build -o goapp .

# Exponer el puerto de la app
EXPOSE 8080

# Ejecutar la aplicación
CMD ["/app/goapp"]