![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![Flutter](https://img.shields.io/badge/Flutter-02569B?style=for-the-badge&logo=flutter&logoColor=white)
![AWS IoT Core](https://img.shields.io/badge/AWS_IoT_Core-FF9900?style=for-the-badge&logo=amazonaws&logoColor=white)
![AWS Lambda](https://img.shields.io/badge/AWS_Lambda-FF9900?style=for-the-badge&logo=awslambda&logoColor=white)
![Amazon DynamoDB](https://img.shields.io/badge/Amazon_DynamoDB-4053D6?style=for-the-badge&logo=amazondynamodb&logoColor=white)
![Amazon API Gateway](https://img.shields.io/badge/Amazon_API_Gateway-FF4F8B?style=for-the-badge&logo=amazonapigateway&logoColor=white)
![Amazon Cognito](https://img.shields.io/badge/Amazon_Cognito-DD344C?style=for-the-badge&logo=amazoncognito&logoColor=white)

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
