cmake_minimum_required(VERSION 3.16)

# Nome do projeto
project(ClipboardMonitor VERSION 1.0 LANGUAGES CXX)

# Configurações de compilação
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontre o pacote Qt necessário
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(X11 REQUIRED)

# Adicione os arquivos do projeto
set(SOURCES
    main.cpp
    src/clipboard-monitor-window.cpp
    # src/keyboard-monitor.cpp
)

set(HEADERS
    src/clipboard-monitor-window.h
    src/keyboard-monitor.h
)

# Adicione ao projeto o suporte ao Meta-Object Compiler (MOC) e ao recurso Qt.
qt_wrap_cpp(MOC_HEADERS ${HEADERS})

# Crie o executável
add_executable(${PROJECT_NAME} ${SOURCES} ${MOC_HEADERS})

# Vincule o executável às bibliotecas do Qt
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Gui Qt6::Widgets X11)

# Configuração padrão para projetos Qt
qt_standard_project_setup()

# Inclua os diretórios para as bibliotecas e cabeçalhos do Qt
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
