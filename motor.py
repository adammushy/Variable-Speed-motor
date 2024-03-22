import sys
import json
import socket
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QPushButton,
    QComboBox, QLabel, QLineEdit, QTableWidget, QTableWidgetItem, QMessageBox
)
from PyQt5.QtCore import QTimer
from pyqtgraph import PlotWidget
import serial as serial_port
import firebase_admin
from firebase_admin import credentials, firestore

class SensorDataReceiver(QMainWindow):
    def __init__(self):
        super().__init__()
        
        self.setWindowTitle("Sensor Data Receiver")
        self.setFixedSize(830, 620)  # Set fixed window size
        
        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        
        self.layout = QVBoxLayout()
        self.central_widget.setLayout(self.layout)
        self.start_stop_button = QPushButton("Start")
        self.start_stop_button.clicked.connect(self.toggle_start_stop)
        self.layout.addWidget(self.start_stop_button)

        self.com_port_label = QLabel("COM Port:")
        self.layout.addWidget(self.com_port_label)
        self.com_port_combobox = QComboBox()
        self.com_port_combobox.addItems(["COM{}".format(i) for i in range(1, 10)])
        self.layout.addWidget(self.com_port_combobox)
        
        self.ip_label = QLabel("IP Address:")
        self.layout.addWidget(self.ip_label)
        self.ip_entry = QLineEdit()
        self.layout.addWidget(self.ip_entry)
        
        self.port_label = QLabel("Port:")
        self.layout.addWidget(self.port_label)
        self.port_entry = QLineEdit()
        self.layout.addWidget(self.port_entry)
        
        button_layout = QHBoxLayout()
        
        self.save_ip_button = QPushButton("Save IP Address")
        self.save_ip_button.clicked.connect(self.save_ip_address)
        button_layout.addWidget(self.save_ip_button)
        
        self.generate_json_button = QPushButton("Generate JSON")
        self.generate_json_button.clicked.connect(self.generate_json)
        button_layout.addWidget(self.generate_json_button)
        
        self.send_json_button = QPushButton("Send JSON Data")
        self.send_json_button.clicked.connect(self.send_json_data)
        button_layout.addWidget(self.send_json_button)
        
        self.layout.addLayout(button_layout)
        
        self.graphs_layout = QHBoxLayout()
        self.layout.addLayout(self.graphs_layout)
        
        self.graphs = {}
        self.plot_curve = {}
        self.plot_data = {}
        self.data = { "temperature": None, "bottles": None,"filltimer": None    }
        
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_data)
        self.is_running = False
        
        self.create_graphs()
        
        self.table_widget = QTableWidget()
        self.table_widget.setRowCount(6)
        self.table_widget.setColumnCount(2)
        self.table_widget.setHorizontalHeaderLabels(["Sensor", "Value"])
        self.layout.addWidget(self.table_widget)
        
        self.sent_success_msg = QMessageBox()
        self.sent_success_msg.setIcon(QMessageBox.Information)
        self.sent_success_msg.setText("Data sent successfully!")
        self.sent_failed_msg = QMessageBox()
        self.sent_failed_msg.setIcon(QMessageBox.Warning)
        self.sent_failed_msg.setText("Failed to send data!")
        
        self.saved_ip_address = ""
        self.serial = None

        # Firebase setup
        cred = credentials.Certificate("C:/Users/Mastra/Desktop/whatsapp/electronics-34416-firebase-adminsdk-j2xvu-8386ab7f85.json")
        print("Initializing Firebase...")
        
        firebase_admin.initialize_app(cred)
        print("Initialized...")
        
        self.db = firestore.client()
        print("Initialization DOne...")
        


    def create_graphs(self):
        for sensor in self.data.keys():
            graph_layout = QVBoxLayout()
            self.graphs_layout.addLayout(graph_layout)
            
            self.graphs[sensor] = PlotWidget()
            graph_layout.addWidget(self.graphs[sensor])
            
            self.graphs[sensor].setBackground('w')
            self.graphs[sensor].setTitle(sensor)
            self.graphs[sensor].showGrid(x=True, y=True)
            self.graphs[sensor].setYRange(0, 100)  # Adjust as needed
            
            self.plot_data[sensor] = []
            self.plot_curve[sensor] = self.graphs[sensor].plot(self.plot_data[sensor], name=sensor)
        
    def toggle_start_stop(self):
        if not self.is_running:
            self.start_timer()
        else:
            self.stop_timer()
        
    def start_timer(self):
        self.start_stop_button.setText("Stop")
        self.is_running = True
        self.timer.start(1000)  # Update every 1 second
        
        # Open serial port
        com_port = self.com_port_combobox.currentText()
        self.serial = serial_port.Serial(com_port, 9600)  # Adjust baud rate as needed
    
    def stop_timer(self):
        self.start_stop_button.setText("Start")
        self.is_running = False
        self.timer.stop()
        
        # Close serial port
        if self.serial is not None:
            self.serial.close()
        
    def update_data(self):
        if self.serial is not None and self.serial.is_open:
            # Read data from serial port
            data_received = self.serial.readline().decode().strip()
            print("Received:", data_received)

            # Process received data
            readings = data_received.split(";")
            if len(readings) == 4:
                for i, (sensor, value) in enumerate(zip(self.data.keys(), readings)):
                    if sensor != "Device ID":  # Skip the "Device ID" sensor
                        self.data[sensor] = float(value)
                        self.table_widget.setItem(i, 0, QTableWidgetItem(sensor))
                        self.table_widget.setItem(i, 1, QTableWidgetItem(value))
                        self.plot_data[sensor].append(float(value))
                        self.plot_curve[sensor].setData(self.plot_data[sensor])
                self.send_data_to_firebase(self.data)
        else:
            print("Serial port is not open!")
                
    def save_ip_address(self):
        self.saved_ip_address = self.ip_entry.text()
        if self.saved_ip_address:
            print("IP Address saved:", self.saved_ip_address)
        else:
            print("Please enter an IP address.")
            
    def send_json_data(self):
        if self.saved_ip_address:
            try:
                port = int(self.port_entry.text())
                self.send_data_to_server(self.saved_ip_address, port, self.data)
                self.sent_success_msg.exec_()
            except ValueError:
                print("Invalid port number:", port_text)
                self.sent_failed_msg.exec_()
        else:
            print("No IP address saved.")
            self.sent_failed_msg.exec_()
        
    def send_data_to_server(self, ip, port, data):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((ip, port))
                data_to_send = json.dumps(data)
                s.sendall(data_to_send.encode())
                print("Data sent successfully!")
        except Exception as e:
            print("Error sending data:", e)
    
    def generate_json(self):
        # Generate JSON string with sensor data
        json_data = json.dumps(self.data)
        print(json_data)

    def send_data_to_firebase(self, data):
        try:
            # Replace "your_collection" with your Firestore collection name
            doc_ref = self.db.collection("data").document("motordata")
            doc_ref.set(data)
            print("Data sent to Firebase successfully!")
        except Exception as e:
            print("Error sending data to Firebase:", e)

# The rest of your code remains the same

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = SensorDataReceiver()
    window.show()
    sys.exit(app.exec_())
