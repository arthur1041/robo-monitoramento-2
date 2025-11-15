
# Robot Realtime — Setup & Run (ESP32 + WebSocket server + Web UI + Docker)

Este repositório contém um projeto **robot-realtime** com três partes:
- `esp32/` — firmware ESP-IDF para o ESP32 (código C).
- `server/` — Node.js WebSocket relay (`index.js`).
- `web/` — front-end estático (`index.html`).
- `docker-compose.yml` — orquestra `server` e `web`.

---

## Sumário rápido (comandos principais)

```bash
# Rodar server + web em Docker
docker compose up --build -d

# Ver logs do server
docker compose logs -f server

# Compilar & flashar ESP (host)
cd esp32
idf.py build
idf.py -p /dev/ttyACM0 flash monitor

# Parar containers
docker compose down
```

---

## 0) Requisitos

- Docker + Docker Compose  
- Git  
- ESP-IDF v5.3.1  
- Node.js (opcional; Docker já resolve para server/web)
- Acesso à porta serial (`/dev/ttyUSB*` ou `/dev/ttyACM*`)

Links:
- Docker: https://docs.docker.com/engine/install  
- ESP-IDF: https://docs.espressif.com/projects/esp-idf/en/latest/esp-idf/get-started/

---

## 1) Instalar Docker (Ubuntu)

```bash
sudo usermod -aG docker $USER
newgrp docker
docker ps
```

---

## 2) Instalar ESP-IDF

```bash
cd $HOME
git clone --branch v5.3.1 https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
. ./export.sh
```

Testar:

```bash
idf.py --version
```

---

## 3) Configurar acesso à porta serial

```bash
sudo usermod -aG dialout $USER
newgrp dialout
```

---

## 4) Obter o IP da máquina (para colocar no ESP)

O ESP32 precisa se conectar ao **seu computador**, não ao container.  
Use:

```bash
ip route get 1.1.1.1
hostname -I
```

Exemplo de saída:

```
1.1.1.1 via 192.168.0.1 dev wlp0s20f3 src 192.168.0.107
```

O IP que importa é o `src` → **192.168.0.107**.

No código, substitua:

```c
#define WS_SERVER_URI "ws://IP_MAQUINA:8080"
```

Exemplo:

```c
#define WS_SERVER_URI "ws://192.168.0.107:8080"
```

---

## 5) Configurar Wi‑Fi no ESP32

No arquivo `esp32/main/main.c`, substitua:

```c
#define WIFI_SSID "NOME_DA_REDE"
#define WIFI_PASS "SENHA_DA_REDE"
```

Pelo nome e senha reais:

```c
#define WIFI_SSID "TP-Link_8CDA"
#define WIFI_PASS "29587315"
```

---

## 6) Rodar o servidor WebSocket + Web UI com Docker

Na raiz do projeto:

```bash
docker compose up --build -d
```

Logs do servidor:

```bash
docker compose logs -f server
```

A UI ficará disponível em:

```
http://localhost:3001
http://IP_MAQUINA:3001
```

---

## 7) Compilar e gravar o firmware no ESP32

Entre na pasta:

```bash
cd esp32
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

Para descobrir a porta:

```bash
ls /dev/ttyUSB* /dev/ttyACM*
dmesg | tail
```

Para sair do monitor: **Ctrl+]**

---

## 8) Fluxo de teste completo

1. Atualize o `main.c` com:
   - SSID e senha
   - `WS_SERVER_URI` com `ws://IP_MAQUINA:8080`

2. Rode os containers:
   ```bash
   docker compose up --build -d
   ```

3. Veja logs do server:
   ```bash
   docker compose logs -f server
   ```

4. Compile e grave o ESP32:
   ```bash
   cd esp32
   idf.py build
   idf.py -p /dev/ttyACM0 flash monitor
   ```

5. No monitor do ESP você deve ver:
   ```
   Got IP: 192.168.0.xxx
   Websocket connected
   ```

6. Abra a UI e envie comandos.

---

## 9) Instruções para Windows

Você tem duas opções:

### ✔ Opção A — Rodar tudo no Windows puro (sem WSL)
- Instale Docker Desktop  
- Instale ESP-IDF para Windows (instalador oficial `.exe`)  
- A porta do ESP32 aparecerá como `COM3`, `COM4`, etc.

Para flashar:

```powershell
idf.py -p COM4 flash monitor
```

O IP da máquina:

```powershell
ipconfig
```

Use o IP da interface Wi‑Fi, exemplo:

```
IPv4 Address. . . . . . . . . . . : 192.168.0.107
```

No `main.c`:

```c
#define WS_SERVER_URI "ws://192.168.0.107:8080"
```

---

### ✔ Opção B — Usar WSL2
1. Instalar Docker Desktop com integração WSL2.
2. Instalar ESP-IDF dentro do WSL2.
3. Mapear porta USB do Windows para o WSL:

No PowerShell admin:

```powershell
usbipd wsl list
usbipd wsl attach --busid <BUSID>
```

No WSL:

```bash
ls /dev/ttyS*
idf.py -p /dev/ttyS4 flash monitor
```

---

## 10) Troubleshooting

### ESP mostra: **Host unreachable**
- IP errado no `WS_SERVER_URI`.
- ESP e PC em redes diferentes.
- Firewall bloqueando porta 8080.

### Server fica reiniciando no Docker
- Volumes sobrescrevendo `node_modules`.  
  Solução: usar volume nomeado ou remover volume `server_node_modules`.

### Sem permissão na porta serial
```bash
sudo usermod -aG dialout $USER
newgrp dialout
```

---

Pronto! Seu ambiente está configurado.