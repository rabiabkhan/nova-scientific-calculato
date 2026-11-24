# Nova Scientific Calculator (C++ Backend)

A beginner-friendly scientific calculator. The browser UI is HTML/CSS/JavaScript; **all expression evaluation is performed by the C++ HTTP server**.

## Features
- Basic arithmetic, brackets, remainder and powers
- sin, cos, tan, sqrt, log, ln, abs
- DEG/RAD mode, keyboard input, theme toggle and recent history
- Responsive layout

## Run locally (Linux/macOS)
```bash
g++ -std=c++17 -O2 calculator_server.cpp -o calculator_server
./calculator_server
```
Open http://localhost:8080

## Deploy free on Render
1. Create a GitHub account and a new **public** repository.
2. Upload all files and folders from this project. Keep `Dockerfile`, `render.yaml`, `calculator_server.cpp`, and `public/` at the repository root.
3. Sign in to https://render.com/ using GitHub.
4. Choose **New + → Blueprint** and select your repository (or choose **New + → Web Service**, connect the repository, and select Docker).
5. Confirm the service uses the Dockerfile and the Free instance type if offered, then deploy.
6. Wait for the deployment to finish. Open the `onrender.com` URL shown by Render.

The app reads Render's `PORT` environment variable. Render free services may spin down when idle, so the first visit after inactivity can take a little time.

## API
`POST /api/calculate`
```json
{"expression":"sin(30)+sqrt(81)","degrees":true}
```
Returns `{"ok":true,"result":9.5}`.

## Notes
- History is kept in the current browser tab/session only; it is not stored on a server.
- This is an educational demo, not a financial or safety-critical calculator.
