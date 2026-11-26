![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![Flutter](https://img.shields.io/badge/Flutter-02569B?style=for-the-badge&logo=flutter&logoColor=white)
![AWS](https://img.shields.io/badge/AWS-FF9900?style=for-the-badge)

```mermaid
flowchart LR
    ESP["ESP32"]
    APP["Flutter App"]
    IOT["IoT Core"]
    LAMBDA["Lambda (Backend)"]
    DB["DynamoDB (Storage)"]
    API["API Gateway"]
    AUTH["Cognito"]

    ESP -->|Cache| ESP
    ESP -->|MQTTS| IOT
    IOT <--> LAMBDA
    LAMBDA <--> DB
    APP -->|HTTPS| API
    API --> LAMBDA
    APP <-->|JWT| AUTH
```
