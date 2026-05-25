import serial
import struct
import numpy as np
import matplotlib.pyplot as plt

PORTA = "COM6"
BAUD = 115200

CMD_RECEIVE_INT = 1

N = 200

#
# gera senoide
#
t = np.arange(N)

onda = 2048 + 1800*np.sin(
    2*np.pi*t/N
)

onda = onda.astype(np.uint16)

#
# abre serial
#
ser = serial.Serial(
    PORTA,
    BAUD,
    timeout=5
)

#
# cabeçalho
#
header = struct.pack(
    "<BH",
    CMD_RECEIVE_INT,
    N*2
)

print("Enviando vetor...")

ser.write(header)

#
# envia amostras
#
for valor in onda:
    ser.write(
        struct.pack(
            "<H",
            int(valor)
        )
    )

print("Aguardando retorno ADC...")

dados = []

for i in range(N):

    rx = ser.read(2)

    if len(rx) < 2:
        print("Timeout")
        break

    valor = struct.unpack(
        "<H",
        rx
    )[0]

    dados.append(valor)

ser.close()

dados = np.array(dados)

#
# FFT
#
fft_adc = np.abs(
    np.fft.rfft(dados)
)

freq = np.arange(
    len(fft_adc)
)

#
# gráficos
#
plt.figure(figsize=(10,8))

plt.subplot(2,1,1)

plt.plot(onda,label="Enviado")
plt.plot(dados,label="ADC")

plt.title("Dominio do Tempo")
plt.grid(True)
plt.legend()

plt.subplot(2,1,2)

plt.plot(freq,fft_adc)

plt.title("FFT")
plt.grid(True)

plt.tight_layout()

plt.show()