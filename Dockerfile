# Multi-stage Dockerfile for C++ Custom Memory Allocator & Visualizer Server
FROM gcc:latest AS builder

WORKDIR /app
COPY . .

# Build the C++ executable for Linux
RUN make

# Production image
FROM ubuntu:latest

RUN apt-get update && apt-get install -y libstdc++6 && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/main .
COPY --from=builder /app/webui ./webui

EXPOSE 8080

CMD ["./main"]
