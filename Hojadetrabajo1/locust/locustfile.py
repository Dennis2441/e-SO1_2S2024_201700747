from locust import HttpUser, task, between

class TrafficUser(HttpUser):
    wait_time = between(1, 5)

    @task
    def send_json_data(self):
        self.client.post("/endpoint", json={"input": "test_input", "disco": "test_disco"})