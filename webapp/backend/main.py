# app.py
from flask import Flask, request, jsonify
from datetime import datetime
import sqlite3
from flask_cors import CORS

app = Flask(__name__)
CORS(app)


# Database initialization
def init_db():
    conn = sqlite3.connect("iot_data.db")
    c = conn.cursor()

    # Create tables
    c.execute(
        """
        CREATE TABLE IF NOT EXISTS sensor_data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            temperature REAL,
            humidity REAL,
            light_level INTEGER,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    """
    )

    c.execute(
        """
        CREATE TABLE IF NOT EXISTS system_settings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            temp_threshold REAL,
            moisture_threshold INTEGER,
            light_threshold INTEGER,
            operation_mode TEXT,
            last_updated DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    """
    )

    # Insert default settings if none exist
    c.execute("SELECT COUNT(*) FROM system_settings")
    if c.fetchone()[0] == 0:
        c.execute(
            """
            INSERT INTO system_settings 
            (temp_threshold, moisture_threshold, light_threshold, operation_mode)
            VALUES (23.0, 50, 1200, 'AUTO')
        """
        )

    conn.commit()
    conn.close()


init_db()


# Helper function to get database connection
def get_db():
    conn = sqlite3.connect("iot_data.db")
    conn.row_factory = sqlite3.Row
    return conn


# API Endpoints


@app.route("/api/sensor-data", methods=["POST"])
def post_sensor_data():
    data = request.json
    conn = get_db()
    c = conn.cursor()

    c.execute(
        """
        INSERT INTO sensor_data 
        (temperature, humidity, light_level)
        VALUES (?, ?, ?)
    """,
        (
            data.get("temperature"),
            data.get("humidity"),
            data.get("light_level"),
        ),
    )

    conn.commit()
    conn.close()

    return jsonify({"status": "success"})


@app.route("/api/sensor-data", methods=["GET"])
def get_sensor_data():
    limit = request.args.get("limit", default=10, type=int)
    conn = get_db()
    c = conn.cursor()

    c.execute(
        """
        SELECT * FROM sensor_data 
        ORDER BY timestamp DESC 
        LIMIT ?
    """,
        (limit,),
    )

    data = [dict(row) for row in c.fetchall()]
    conn.close()

    return jsonify(data)


@app.route("/api/settings", methods=["GET"])
def get_settings():
    conn = get_db()
    c = conn.cursor()

    c.execute("SELECT * FROM system_settings ORDER BY id DESC LIMIT 1")
    settings = dict(c.fetchone())
    conn.close()

    return jsonify(settings)


@app.route("/api/settings", methods=["POST"])
def update_settings():
    data = request.json
    conn = get_db()
    c = conn.cursor()

    c.execute(
        """
        INSERT INTO system_settings 
        (temp_threshold, moisture_threshold, light_threshold, operation_mode)
        VALUES (?, ?, ?, ?)
    """,
        (
            data.get("temp_threshold"),
            data.get("moisture_threshold"),
            data.get("light_threshold"),
            data.get("operation_mode"),
        ),
    )

    conn.commit()
    conn.close()

    return jsonify({"status": "success"})


@app.route("/api/mode", methods=["POST"])
def set_mode():
    data = request.json
    mode = data.get("mode")

    if mode not in ["AUTO", "MANUAL", "OFF"]:
        return jsonify({"error": "Invalid mode"}), 400

    conn = get_db()
    c = conn.cursor()

    c.execute(
        """
        UPDATE system_settings 
        SET operation_mode = ?, 
            last_updated = CURRENT_TIMESTAMP 
        WHERE id = (SELECT id FROM system_settings ORDER BY id DESC LIMIT 1)
    """,
        (mode,),
    )

    conn.commit()
    conn.close()

    return jsonify({"status": "success"})


if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0", port=5001)
