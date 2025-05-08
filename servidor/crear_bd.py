# crear_base_datos.py
import sqlite3

# Crear conexión y cursor
conn = sqlite3.connect('sensores.db')
c = conn.cursor()

# Crear tabla de datos si no existe
c.execute('''
    CREATE TABLE IF NOT EXISTS datos (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        fecha TEXT,
        temperatura REAL,
        humedad REAL,
        lluvia INTEGER
    )
''')

# Crear tabla de configuración si no existe
c.execute('''
    CREATE TABLE IF NOT EXISTS config (
        clave TEXT PRIMARY KEY,
        valor TEXT
    )
''')

# Insertar valores predeterminados solo si no existen ya
config_default = {
    'temp_umbral': '30',
    'hum_umbral': '70',
    'lluvia_umbral': '900',
    'correo_destino': 'tucorreo@ejemplo.com'
}

for clave, valor in config_default.items():
    c.execute('INSERT OR IGNORE INTO config (clave, valor) VALUES (?, ?)', (clave, valor))

# Confirmar y cerrar
conn.commit()
conn.close()

print("Base de datos y configuración inicial listas.")
