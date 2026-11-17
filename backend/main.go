package main

import (
	"context"
	"encoding/json"
	"fmt"
	"log"
	"os"

	"github.com/aws/aws-lambda-go/lambda"
	"github.com/aws/aws-sdk-go-v2/aws"
	"github.com/aws/aws-sdk-go-v2/config"
	"github.com/aws/aws-sdk-go-v2/service/iotdataplane"
)

type CardRead struct {
	UID       string `json:"uid"`
	ThingName string `json:"thing_name"`
}

type Decision struct {
	UID      string `json:"uid"`
	Decision string `json:"decision"`
}

func main() {
	cfg, err := config.LoadDefaultConfig(context.Background())
	if err != nil {
		log.Fatal(err)
	}

	iot := iotdataplane.NewFromConfig(cfg, func(o *iotdataplane.Options) {
		o.BaseEndpoint = aws.String("https://" + os.Getenv("IOT_ENDPOINT"))
	})

	lambda.Start(func(ctx context.Context, card CardRead) error {
		if card.UID == "" || card.ThingName == "" {
			return fmt.Errorf("missing UID or thing name")
		}

		decision := "no"

		if card.UID == "1234567" {
			decision = "yes"
		}

		payload, err := json.Marshal(Decision{
			UID:      card.UID,
			Decision: decision,
		})

		if err != nil {
			return err
		}

		topic := fmt.Sprintf("doors/%s/decision", card.ThingName)
		_, err = iot.Publish(ctx, &iotdataplane.PublishInput{
			Topic:   aws.String(topic),
			Payload: payload,
			Qos:     1,
		})
		return err
	})
}
