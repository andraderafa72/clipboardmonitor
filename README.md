# Clipboard Monitor

Plain-text clipboard history (like **Win + V**). **Super + V** opens recent snippets to paste again.

**Limits:** **X11** global hotkey (`XGrabKey`); **pure Wayland** may not get **Super + V**. History is **text only** (`QClipboard::text()`).

**Features:** Up to **500** entries, XDG store at `~/.local/share/clipboardmonitor/history.txt`, tray icon, single instance per user.

**Stack:** Qt6 Widgets, X11.

## Compatibility

| | |
| --- | --- |
| **OS** | **Linux** with a graphical session. Not aimed at Windows or macOS. |
| **Distros** | Any distro that ships **Qt 6** (Widgets), **CMake**, **libX11**, and a **C++17** compiler. Examples you can package against: **Arch**, **Debian**, **Ubuntu**, **Fedora**, **openSUSE**, **Gentoo**; minimal distros like **Alpine** work if you install the same deps and run an **X11** session. |
| **Session** | **X11** or **XWayland** for the shortcut; session autostart (not a system `systemd` service). |

Package names vary, for example: `qt6-base` / `libx11` (Arch), `qt6-base-dev` `libx11-dev` (Debian/Ubuntu), `qt6-qtbase-devel` `libX11-devel` (Fedora).

## Build / install

Needs: CMake, Qt6 Widgets, libX11, C++17.

```bash
chmod +x ./build.sh   # optional
./build.sh
# or: cmake -S . -B build && cmake --build build
# binary: build/clipboardmonitor
```

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build && cmake --install build
```

Installs the binary, `share/icons/hicolor/scalable/apps/clipboard-monitor.svg`, and `share/applications/clipboard-monitor.desktop`.

## Autostart

GUI + X11 must be up: use **`~/.config/autostart/`** (or **Settings → Startup**), not a system service.

```bash
ln -sf /usr/local/share/applications/clipboard-monitor.desktop ~/.config/autostart/
```

Template: [data/clipboard-monitor.desktop](data/clipboard-monitor.desktop). Fix **`Exec=`** if the binary is not on `PATH`. On mixed Wayland/X11 stacks you may need `QT_QPA_PLATFORM=xcb` in `Exec=` or a wrapper.

## Contributing

Pull requests welcome.

## Future work

Pin/remove entries, image preview, search.
