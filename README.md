# Clipboard Monitor

O **Clipboard Monitor** registra entradas de **texto** copiadas ou cortadas e mantém um histórico, de forma parecida ao atalho `Win + V` do Windows. Ao pressionar **Super + V**, a janela lista os trechos recentes para colar de novo.

## Limitações

- **Sessão X11**: o atalho global usa `XGrabKey`. Em **Wayland** puro o grab não se comporta igual; use sessão X11 ou XWayland conforme seu ambiente.
- **Somente texto**: o histórico usa `QClipboard::text()` (sem imagens ou rich text).

## Funcionalidades

- Histórico de clipboard em texto, persistido em `~/.local/share/clipboardmonitor/history.txt` (XDG).
- Até **500** entradas; entradas mais antigas saem da lista e do ficheiro quando o limite é ultrapassado.
- Bandeja do sistema com ícone embutido (fallback se o tema não tiver ícone instalado).
- Uma instância por utilizador (ficheiro de lock em `XDG_RUNTIME_DIR` ou diretório temporário).

## Tecnologias

- **Qt6** (Widgets)
- **X11** para o atalho **Super + V**

## Compilação

Requisitos: CMake, Qt6 (Widgets), libX11, compilador C++17.

### `build.sh`

```bash
chmod +x ./build.sh   # se necessário
./build.sh
```

### Manual

```bash
cmake -S . -B build
cmake --build build
```

O executável fica em `build/clipboardmonitor`. Opcionalmente copie para `dist/` como antes.

### Instalação (prefixo configurável)

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
cmake --install build
```

Isto instala o binário, o ícone em `share/icons/hicolor/scalable/apps/clipboard-monitor.svg`, o `.desktop` em `share/applications` e uma unidade **systemd user** gerada com o caminho `ExecStart` correto em `lib/systemd/user/clipboard-monitor.service`.

## Autostart

### Entrada `.desktop` (recomendado em muitos ambientes)

Após `cmake --install`, copie ou faça ligação simbólica:

```bash
ln -sf /usr/local/share/applications/clipboard-monitor.desktop ~/.config/autostart/
```

(Ajuste o prefixo se não for `/usr/local`.)

O repositório inclui o modelo em [data/clipboard-monitor.desktop](data/clipboard-monitor.desktop) (`Exec=clipboardmonitor` — o binário deve estar no `PATH` ou edite o `Exec` com caminho completo).

### systemd (utilizador)

O `ExecStart` no ficheiro gerado corresponde ao prefixo do CMake (`-DCMAKE_INSTALL_PREFIX`). Copie a unidade para `~/.config/systemd/user/` e active:

```bash
mkdir -p ~/.config/systemd/user
cp build/clipboard-monitor.service ~/.config/systemd/user/
systemctl --user daemon-reload
systemctl --user enable --now clipboard-monitor.service
```

Depois de `cmake --install`, pode copiar de `lib/systemd/user/clipboard-monitor.service` sob o prefixo em vez da cópia em `build/`, se o systemd listar esse diretório; caso contrário mantenha `~/.config/systemd/user/`.

**Não** é recomendado um serviço **system** com `User=` e `DISPLAY=:0` fixo: corre o risco de arrancar antes do X11 e exige variáveis da sessão. Prefira **systemd --user** ou autostart da sessão gráfica.

### `QT_QPA_PLATFORM`

Se necessário forçar o backend X11 num ambiente misto: `export QT_QPA_PLATFORM=xcb` antes do `Exec` (opcional, documente no vosso `.desktop` personalizado).

## Contribuições

Sinta-se à vontade para fazer fork e enviar pull requests.

## Implementações futuras

- Fixar itens no histórico;
- Remover um único item;
- Pré-visualização de imagens (requer mudar o modelo de dados);
- Pesquisa no histórico;
