# Patient-Management-System-using-ESP8266-and-RFID
This Node.js Express server powers a secure patient record system designed for fast and encrypted medical record access using unique identifiers (UIDs). Built for integration with NFC-based healthcare systems, it supports patient registration, search, retrieval, and deletion functionalities.

🚀 Features

📡 UID-based Patient Identification
🔐 UID Encryption Support (via RSA)
🗂️ MongoDB Integration for persistent medical data

🧾 Patient CRUD operations:

Register or update patient details
Search patients by name
View patient details by UID
Delete patient records
🌐 Static file serving for frontend pages
📈 Real-time UID polling endpoint

🛠️ Tech Stack

Node.js + Express.js
MongoDB + Mongoose
Body-parser for form handling
Path module for directory navigation

📁 File Highlights

server.js: Core server logic and API routes
uid.js: UID storage utility
rsa.js: Handles encryption of UID data

