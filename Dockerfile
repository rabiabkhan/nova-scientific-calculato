FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends g++ && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY calculator_server.cpp .
COPY public ./public
RUN g++ -std=c++17 -O2 -Wall calculator_server.cpp -o calculator_server
ENV PORT=10000
EXPOSE 10000
CMD ["./calculator_server"]
