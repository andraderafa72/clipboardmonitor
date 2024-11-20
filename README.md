# Clipboard Monitor

O **Clipboard Monitor** é uma aplicação que registra todas as entradas copiadas ou cortadas pelo usuário, funcionando de maneira similar à ferramenta de histórico de área de transferência do Windows ao pressionar `Win + V`. A aplicação permite ao usuário visualizar e acessar os itens copiados anteriormente através de uma janela que é exibida ao pressionar `Super + V`.

## Funcionalidades

- **Registro de Clipboard**: A aplicação monitora tudo o que o usuário copia ou corta (texto, imagens, etc.) e mantém um histórico dessas entradas.
- **Exibição de Histórico**: Ao pressionar `Super + V`, a aplicação exibe uma janela com todos os registros copiados, permitindo que o usuário escolha o item desejado.
- **Integração com X11**: Utiliza a API do X11 para capturar eventos do teclado e interagir com a área de transferência.

## Tecnologias

- **Qt6**: Framework utilizado para construir a interface gráfica e lidar com eventos.
- **X11 API**: Para captura de eventos de teclado (como `Super + V`) e monitoramento da área de transferência.

## Compilação
Requisitos:
- [cmake](https://cmake.org/)
- [git](https://git-scm.com/)

### Etapas da compilação
1. Clone o repositório:
```bash
git clone https://github.com/andraderafa72/clipboardmonitor
```

2. Acesse o repositório clonado e faça a build:

#### Executando `build.sh`:
```bash
chamod +x ./build.sh # se necessário
./build.sh
```

#### Compilando manualmente

```bash
cmake -S . -B build
cmake --build build
mv build/clipboardmonitor dist/
```

3. Execute a aplicação:
```bash
./dist/clipboardmonitor
```

## Adicionando como um serviço no linux

### Crie o arquivo de serviço:
 
```bash
sudo nano /etc/systemd/system/clipboardmonitor.service
```

### Adicione o seguinte conteúdo:

```ini
[Unit]
Description=Clipboard Monitor
After=graphical.target

[Service]
ExecStart=/caminho/para/o/executavel/dist/clipboardmonitor
Restart=always
User=seu_usuario
Environment=DISPLAY=:0
Environment=XAUTHORITY=/home/seu_usuario/.Xauthority

[Install]
WantedBy=multi-user.target
```

### Habilite a aplicação como serviço:
 
```bash
sudo systemctl enable clipboardmonitor.service
sudo systemctl start clipboardmonitor.service
```

#### Verifique se o serviço está habilitado:

```bash
sudo systemctl status clipboardmonitor.service
```

## Contribuições
Se você gostaria de contribuir para o desenvolvimento dessa aplicação, sinta-se à vontade para fazer um fork e enviar um pull request.

----

A aplicação foi projetada para ser leve e fácil de usar, integrando-se de forma fluida com o sistema operacional. Ela monitora a área de transferência e captura eventos de teclado com a ajuda do Qt6 e da API X11, permitindo uma experiência de usuário mais produtiva.

## Implementações futuras

- Fixar itens copiados;
- Remover um único item copiado;
- Previsualizar imagens copiadas;
- Buscar textos copiados;