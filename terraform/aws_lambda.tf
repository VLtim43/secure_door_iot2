locals {
  card_read_zip = "${path.module}/../backend/build/card_read.zip"
}

resource "aws_iam_role" "card_read_lambda" {
  name = "${var.thing_name}-card-read-lambda"

  assume_role_policy = jsonencode({
    Version = "2012-10-17"
    Statement = [{
      Effect    = "Allow"
      Action    = "sts:AssumeRole"
      Principal = { Service = "lambda.amazonaws.com" }
    }]
  })
}

resource "aws_iam_role_policy_attachment" "card_read_logs" {
  role       = aws_iam_role.card_read_lambda.name
  policy_arn = "arn:aws:iam::aws:policy/service-role/AWSLambdaBasicExecutionRole"
}

resource "aws_iam_role_policy" "card_read_publish" {
  name = "publish-card-decision"
  role = aws_iam_role.card_read_lambda.id

  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [{
      Effect   = "Allow"
      Action   = "iot:Publish"
      Resource = "arn:aws:iot:${data.aws_region.current.name}:${data.aws_caller_identity.current.account_id}:topic/doors/${var.thing_name}/decision"
    }]
  })
}

resource "aws_lambda_function" "card_read" {
  function_name    = "${var.thing_name}-card-read"
  role             = aws_iam_role.card_read_lambda.arn
  filename         = local.card_read_zip
  source_code_hash = fileexists(local.card_read_zip) ? filebase64sha256(local.card_read_zip) : null
  handler          = "bootstrap"
  runtime          = "provided.al2023"
  architectures    = ["arm64"]
  timeout          = 10

  environment {
    variables = {
      IOT_ENDPOINT = data.aws_iot_endpoint.mqtt.endpoint_address
    }
  }

  depends_on = [
    aws_iam_role_policy_attachment.card_read_logs,
    aws_iam_role_policy.card_read_publish,
  ]
}

resource "aws_iot_topic_rule" "card_read" {
  name        = "door_card_read"
  enabled     = true
  sql         = "SELECT *, topic(2) AS thing_name FROM 'doors/${var.thing_name}/card-read'"
  sql_version = "2016-03-23"

  lambda {
    function_arn = aws_lambda_function.card_read.arn
  }
}

resource "aws_lambda_permission" "from_iot" {
  statement_id   = "AllowIoTCardRead"
  action         = "lambda:InvokeFunction"
  function_name  = aws_lambda_function.card_read.function_name
  principal      = "iot.amazonaws.com"
  source_arn     = aws_iot_topic_rule.card_read.arn
  source_account = data.aws_caller_identity.current.account_id
}
