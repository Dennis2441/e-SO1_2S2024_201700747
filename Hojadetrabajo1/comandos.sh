# Verificar clusters
kubectl get nodes

# Contenedor server go
docker build -t server-go .
# Despliegue server go
kubectl apply -f deployment.yaml

# Despliegue locust
kubectl apply -f deployment.yaml
