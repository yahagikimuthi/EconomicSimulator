FROM golang:1.22-alpine

# Cgo を利用する場合においてはgcc や g++ を導入する
RUN apk add --no-cache build-base

WORKDIR /app

EXPOSE 8080

CMD ["sh", "-c", "go mod tidy && go run main.go"]