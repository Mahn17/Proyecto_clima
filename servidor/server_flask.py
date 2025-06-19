# servidor.py
from flask import Flask, request, render_template, redirect
import sqlite3
import smtplib
from datetime import datetime
import pytz
from email.mime.text import MIMEText
import ssl
from datetime import datetime, timedelta

ultimo_envio = datetime.min  

app = Flask(__name__)

def guardar_dato(temp, hum, lluvia):
    conn = sqlite3.connect('sensores.db')
    c = conn.cursor()

    zona = pytz.timezone('America/Hermosillo')
    fecha_local = datetime.now(zona).strftime('%Y-%m-%d %H:%M:%S')

    c.execute('INSERT INTO datos (fecha, temperatura, humedad, lluvia) VALUES (?, ?, ?, ?)',
              (fecha_local, temp, hum, lluvia))
    conn.commit()
    conn.close()


def enviar_correo(temp, hum, lluvia, destinatario):
    remitente = 'a220203714@unison.mx'
    asunto = 'Datos del sensor recibidos'

    if lluvia > 600:
        estado_lluvia = "Sin lluvia"
    elif lluvia > 300:
        estado_lluvia = "Lluvia ligera"
    else:
        estado_lluvia = "Lluvia intensa"

    zona = pytz.timezone('America/Hermosillo')
    fecha_local = datetime.now(zona).strftime('%Y-%m-%d %H:%M:%S')
    
    cuerpo = f'Fecha: {fecha_local}\nTemperatura: {temp}°C\nHumedad: {hum}%\n{estado_lluvia}'

    mensaje = MIMEText(cuerpo)
    mensaje['Subject'] = asunto
    mensaje['From'] = remitente
    mensaje['To'] = destinatario

    try:
        context = ssl.create_default_context()
        servidor = smtplib.SMTP('smtp.office365.com', 587)
        servidor.ehlo()
        servidor.starttls(context=context)
        servidor.ehlo()
        servidor.login(remitente, 'Uv1s7A3ykG')
        servidor.send_message(mensaje)
        servidor.quit()
        print("Correo enviado")
    except Exception as e:
        print("Error al enviar correo:", e)
        
def obtener_config(clave):
    conn = sqlite3.connect('sensores.db')
    c = conn.cursor()
    c.execute('SELECT valor FROM config WHERE clave = ?', (clave,))
    resultado = c.fetchone()
    conn.close()
    return resultado[0] if resultado else None


@app.route('/datos', methods=['POST'])
def recibir():
    global ultimo_envio
    dato = request.form.get('dato')
    if dato:
        # Esperamos algo como: T:24.5,H:60.1,Lluvia:822
        try:
            partes = dato.split(',')
            temp = float(partes[0].split(':')[1])
            hum = float(partes[1].split(':')[1])
            lluvia = int(partes[2].split(':')[1])

            guardar_dato(temp, hum, lluvia)

            # Obtener umbrales desde la base de datos
            temp_umbral_max = float(obtener_config('temp_umbral_max'))
            hum_umbral_max = float(obtener_config('hum_umbral_max'))
            temp_umbral_min = float(obtener_config('temp_umbral_min'))
            hum_umbral_min = float(obtener_config('hum_umbral_min'))
            lluvia_umbral = int(obtener_config('lluvia_umbral'))

            ahora = datetime.now()
            tiempo_transcurrido = (ahora - ultimo_envio).total_seconds()
            print (tiempo_transcurrido)
            if temp >= temp_umbral_max or hum >= hum_umbral_max or temp <= temp_umbral_min or hum <= hum_umbral_min or  (lluvia_umbral == 300 and 300 < lluvia <= 600) or  (lluvia_umbral == 0 and 0 <= lluvia <= 300) or (lluvia_umbral == 600 and lluvia > 600 and tiempo_transcurrido >= 10) :
                correo = obtener_config('correo_destino')
                enviar_correo(temp, hum, lluvia, correo)
                ultimo_envio = ahora

            print(f"Guardado: T={temp} H={hum} Lluvia={lluvia}")
            return "OK", 200
        except Exception as e:
            print("Error procesando dato:", e)
            return "ERROR", 400
    else:
        print("Dato vacío o incorrecto")
        return "ERROR", 400

@app.route('/dashboard')
def dashboard():
    conn = sqlite3.connect('sensores.db')
    c = conn.cursor()

    # Obtener los últimos 100 registros
    c.execute('SELECT fecha, temperatura, humedad, lluvia FROM datos ORDER BY fecha DESC LIMIT 100')
    datos = c.fetchall()

    # Obtener la última fecha registrada
    c.execute('SELECT MAX(fecha) FROM datos')
    ultima_actualizacion = c.fetchone()[0]

    conn.close()

    fechas = [fila[0] for fila in datos][::-1]
    temperaturas = [fila[1] for fila in datos][::-1]
    humedades = [fila[2] for fila in datos][::-1]
    lluvias = [fila[3] for fila in datos][::-1]
    tabla = list(zip(fechas, temperaturas, humedades, lluvias))

    return render_template('dashboard.html',
                           fechas=fechas,
                           temperaturas=temperaturas,
                           humedades=humedades,
                           lluvias=lluvias,
                           tabla=tabla,
                           ultima_actualizacion=ultima_actualizacion)


@app.route('/correo-prueba')
def correo_prueba():
    try:
        # Datos de prueba
        temp = 35.0
        hum = 75.0
        lluvia = 950

        enviar_correo(temp, hum, lluvia)
        return "Correo de prueba enviado con éxito", 200
    except Exception as e:
        return f"Error al enviar correo: {e}", 500

@app.route('/')
def inicio():
    return redirect('/dashboard')



@app.route('/config', methods=['GET', 'POST'])
def config():
    conn = sqlite3.connect('sensores.db')
    c = conn.cursor()
    if request.method == 'POST':
        for clave in ['temp_umbral_max', 'hum_umbral_max', 'temp_umbral_max', 'hum_umbral_max', 'lluvia_umbral', 'correo_destino']:
            nuevo_valor = request.form.get(clave)
            c.execute('UPDATE config SET valor = ? WHERE clave = ?', (nuevo_valor, clave))
        conn.commit()
    c.execute('SELECT clave, valor FROM config')
    configuraciones = dict(c.fetchall())
    conn.close()
    return render_template('config.html', config=configuraciones)

@app.route('/api/datos')
def api_datos():
    conn = sqlite3.connect('sensores.db')
    c = conn.cursor()
    c.execute('SELECT fecha, temperatura, humedad, lluvia FROM datos ORDER BY fecha DESC LIMIT 100')
    datos = c.fetchall()
    conn.close()

    #datos = datos[::-1]  # Ordenar
    fechas = [d[0] for d in datos]
    temperaturas = [d[1] for d in datos]
    humedades = [d[2] for d in datos]
    lluvias = [d[3] for d in datos]

    return {
        "fechas": fechas,
        "temperaturas": temperaturas,
        "humedades": humedades,
        "lluvias": lluvias
    }

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
