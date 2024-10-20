# Proyecto GKE con Locust y Go

Este proyecto despliega una aplicación en Go y Locust en un clúster de Kubernetes utilizando Google Kubernetes Engine (GKE).

## Estructura del Proyecto

- **locust/**: Contiene el archivo de configuración de Locust (`locustfile.py`) y el despliegue en Kubernetes (`deployment.yaml`).
- **server/**: Contiene la aplicación en Go, su Dockerfile y el archivo de despliegue en Kubernetes.

## Pasos para Desplegar

### 1. Configurar el clúster en GKE

1. Accede a Google Cloud Console.
2. Crea un clúster de Kubernetes en GKE.
3. Conéctate al clúster utilizando `gcloud`:

   ```bash
   gcloud container clusters get-credentials [nombre-del-cluster] --zone [zona]
