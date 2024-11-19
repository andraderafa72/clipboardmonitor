cmake_minimum_required(VERSION 3.16)

# Nome do projeto
project(ClipboardMonitor VERSION 1.0 LANGUAGES CXX)

# Configurações de compilação
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontre o pacote Qt necessário
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Adicione os arquivos do projeto
set(SOURCES
    main.cpp
    src/clipboard-monitor-window.cpp
)

set(HEADERS
    src/clipboard-monitor-window.h
)

# Crie o executável
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Vincule o executável às bibliotecas do Qt
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

qt_wrap_cpp(MOC_FILES ${HEADERS})  # Chama o MOC para os arquivos de cabeçalho

target_sources(${PROJECT_NAME} PRIVATE ${MOC_FILES})

# Configuração para usar o MOC do Qt (Meta-Object Compiler)
qt_standard_project_setup()
