package main

import (
    "encoding/json"
    "fmt"
    "log"
    "net/http"
)

type RequestData struct {
    Input string `json:"input"`
    Disco string `json:"disco"`
}

func handler(w http.ResponseWriter, r *http.Request) {
    var data RequestData
    if err := json.NewDecoder(r.Body).Decode(&data); err != nil {
        http.Error(w, err.Error(), http.StatusBadRequest)
        return
    }
    fmt.Fprintf(w, "Received: %+v\n", data)
}

func main() {
    http.HandleFunc("/endpoint", handler)
    log.Println("Server is running on port 8080")
    log.Fatal(http.ListenAndServe(":8080", nil))
}