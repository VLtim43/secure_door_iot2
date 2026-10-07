terraform {
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 5.0"
    }
  }
}

provider "aws" {
  region = var.aws_region
}

variable "aws_region" {
  type    = string
  default = "eu-west-1"
}

variable "thing_name" {
  type    = string
  default = "secure-door-01"
}

data "aws_region" "current" {}
data "aws_caller_identity" "current" {}

data "aws_iot_endpoint" "mqtt" {
  endpoint_type = "iot:Data-ATS"
}

resource "aws_iot_thing" "door" {
  name = var.thing_name
}

resource "aws_iot_certificate" "door" {
  active = true
}

resource "aws_iot_thing_principal_attachment" "door_certificate" {
  thing     = aws_iot_thing.door.name
  principal = aws_iot_certificate.door.arn
}

resource "aws_iot_policy" "door" {
  name = "${var.thing_name}-policy"

  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Effect   = "Allow"
        Action   = "iot:Connect"
        Resource = "arn:aws:iot:${data.aws_region.current.name}:${data.aws_caller_identity.current.account_id}:client/$${iot:Connection.Thing.ThingName}"
        Condition = {
          Bool = {
            "iot:Connection.Thing.IsAttached" = "true"
          }
        }
      },
      {
        Effect   = "Allow"
        Action   = ["iot:Publish", "iot:Receive"]
        Resource = "arn:aws:iot:${data.aws_region.current.name}:${data.aws_caller_identity.current.account_id}:topic/secure-door/$${iot:Connection.Thing.ThingName}/*"
      },
      {
        Effect   = "Allow"
        Action   = "iot:Subscribe"
        Resource = "arn:aws:iot:${data.aws_region.current.name}:${data.aws_caller_identity.current.account_id}:topicfilter/secure-door/$${iot:Connection.Thing.ThingName}/*"
      }
    ]
  })
}

resource "aws_iot_policy_attachment" "door" {
  policy = aws_iot_policy.door.name
  target = aws_iot_certificate.door.arn
}

output "iot_endpoint" {
  value = data.aws_iot_endpoint.mqtt.endpoint_address
}

output "certificate_pem" {
  value     = aws_iot_certificate.door.certificate_pem
  sensitive = true
}

output "private_key" {
  value     = aws_iot_certificate.door.private_key
  sensitive = true
}

