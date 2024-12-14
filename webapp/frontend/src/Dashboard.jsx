import React, { useState, useEffect } from "react";
import {
    LineChart,
    Line,
    XAxis,
    YAxis,
    CartesianGrid,
    Tooltip,
    ResponsiveContainer,
    Legend,
} from "recharts";
import {
    RefreshCw,
    Droplets,
    Thermometer,
    Sun,
    Settings,
    Power,
    X,
} from "lucide-react";

import { API_URL } from "../constants";

// Helper function to format dates
const formatDate = (timestamp) => {
    if (!timestamp) return "";
    const date = new Date(timestamp);
    return date.toLocaleTimeString("en-US", {
        hour: "2-digit",
        minute: "2-digit",
        hour12: true,
    });
};

// Custom tooltip for charts
const CustomTooltip = ({ active, payload, label }) => {
    if (active && payload && payload.length) {
        return (
            <div className="bg-white p-3 border border-gray-200 shadow-lg rounded-lg">
                <p className="text-gray-600">{formatDate(label)}</p>
                {payload.map((entry, index) => (
                    <p
                        key={index}
                        className="text-sm font-semibold"
                        style={{ color: entry.color }}
                    >
                        {entry.name}: {entry.value.toFixed(1)}{" "}
                        {entry.name === "Temperature" ? "°C" : "%"}
                    </p>
                ))}
            </div>
        );
    }
    return null;
};

export default function Dashboard() {
    const [sensorData, setSensorData] = useState([]);
    const [settings, setSettings] = useState({
        temp_threshold: 0,
        light_threshold: 0,
        operation_mode: "",
    });
    const [loading, setLoading] = useState(true);

    const fetchData = async () => {
        try {
            const [dataResponse, settingsResponse] = await Promise.all([
                fetch(`${API_URL}/sensor-data?limit=20`),
                fetch(`${API_URL}/settings`),
            ]);

            const data = await dataResponse.json();
            const settingsData = await settingsResponse.json();

            setSensorData(data.reverse());
            setSettings(settingsData);
            setLoading(false);
        } catch (error) {
            console.error("Error fetching data:", error);
        }
    };

    useEffect(() => {
        fetchData();
        const interval = setInterval(fetchData, 5000); // Fetch every 5 seconds
        return () => clearInterval(interval);
    }, []);

    const updateMode = async (mode) => {
        try {
            await fetch(`${API_URL}/mode`, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ mode }),
            });
            fetchData();
        } catch (error) {
            console.error("Error updating mode:", error);
        }
    };

    const updateSingleSetting = async (key, value) => {
        try {
            const newSettings = { ...settings, [key]: value };
            await fetch(`${API_URL}/settings`, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(newSettings),
            });
            setSettings(newSettings);
        } catch (error) {
            console.error("Error updating setting:", error);
        }
    };

    const getLatestReadings = () => {
        if (sensorData.length === 0) return null;
        return sensorData[sensorData.length - 1];
    };

    const latestData = getLatestReadings();

    if (loading) {
        return (
            <div className="flex items-center justify-center min-h-screen bg-gray-50">
                <div className="text-center">
                    <RefreshCw className="w-12 h-12 animate-spin text-blue-500 mx-auto mb-4" />
                    <p className="text-gray-600">Loading dashboard...</p>
                </div>
            </div>
        );
    }

    return (
        <div className=" min-h-screen bg-gradient-to-br from-gray-50 to-gray-100 p-6">
            <div className="max-w-7xl mx-auto">
                <div className="text-center mb-8">
                    <h1 className="text-3xl font-bold text-gray-800 mb-2">
                        Smart Home Monitor
                    </h1>
                    <p className="text-gray-600">
                        Last updated: {formatDate(latestData?.timestamp)}
                    </p>

                    <div className="mt-6 inline-flex gap-2 bg-white p-1 rounded-lg shadow">
                        <button
                            onClick={() => updateMode("AUTO")}
                            className={`px-6 py-2 rounded-md transition-all duration-200 ${
                                settings.operation_mode === "AUTO"
                                    ? "bg-blue-500 text-white shadow-md"
                                    : "bg-gray-50 text-gray-700 hover:bg-gray-100"
                            }`}
                        >
                            Auto
                        </button>
                        <button
                            onClick={() => updateMode("MANUAL")}
                            className={`px-6 py-2 rounded-md transition-all duration-200 ${
                                settings.operation_mode === "MANUAL"
                                    ? "bg-blue-500 text-white shadow-md"
                                    : "bg-gray-50 text-gray-700 hover:bg-gray-100"
                            }`}
                        >
                            Manual
                        </button>
                        <button
                            onClick={() => updateMode("OFF")}
                            className={`px-6 py-2 rounded-md transition-all duration-200 ${
                                settings.operation_mode === "OFF"
                                    ? "bg-blue-500 text-white shadow-md"
                                    : "bg-gray-50 text-gray-700 hover:bg-gray-100"
                            }`}
                        >
                            Off
                        </button>
                    </div>
                </div>

                {/* Current Readings */}
                <div className="grid grid-cols-1 md:grid-cols-3 gap-6 mb-8">
                    <div className="bg-white p-6 rounded-xl shadow-md hover:shadow-lg transition-shadow duration-200">
                        <div className="flex items-center gap-3 mb-3">
                            <div className="p-2 bg-red-50 rounded-lg">
                                <Thermometer className="text-red-500 w-6 h-6" />
                            </div>
                            <h2 className="text-gray-700 font-semibold">
                                Temperature
                            </h2>
                        </div>
                        <p className="text-3xl font-bold text-gray-800 mb-2">
                            {latestData?.temperature?.toFixed(1)}°C
                        </p>
                        <div className="flex items-center gap-2">
                            <label className="text-sm text-gray-500">
                                Threshold:
                            </label>
                            <input
                                type="number"
                                step="0.1"
                                value={settings.temp_threshold}
                                onChange={(e) =>
                                    updateSingleSetting(
                                        "temp_threshold",
                                        parseFloat(e.target.value)
                                    )
                                }
                                className="w-20 px-2 py-1 text-sm border border-gray-300 rounded-md"
                            />
                            <span className="text-sm text-gray-500">°C</span>
                        </div>
                    </div>

                    <div className="bg-white p-6 rounded-xl shadow-md hover:shadow-lg transition-shadow duration-200">
                        <div className="flex items-center gap-3 mb-3">
                            <div className="p-2 bg-blue-50 rounded-lg">
                                <Droplets className="text-blue-500 w-6 h-6" />
                            </div>
                            <h2 className="text-gray-700 font-semibold">
                                Soil Moisture
                            </h2>
                        </div>
                        <p className="text-3xl font-bold text-gray-800 mb-2">
                            {latestData?.humidity?.toFixed(1)}%
                        </p>
                        <div className="flex items-center gap-2">
                            <label className="text-sm text-gray-500">
                                Threshold:
                            </label>
                            <input
                                type="number"
                                value={settings.moisture_threshold}
                                onChange={(e) =>
                                    updateSingleSetting(
                                        "moisture_threshold",
                                        parseInt(e.target.value)
                                    )
                                }
                                className="w-20 px-2 py-1 text-sm border border-gray-300 rounded-md"
                            />
                            <span className="text-sm text-gray-500">%</span>
                        </div>
                    </div>

                    <div className="bg-white p-6 rounded-xl shadow-md hover:shadow-lg transition-shadow duration-200">
                        <div className="flex items-center gap-3 mb-3">
                            <div className="p-2 bg-yellow-50 rounded-lg">
                                <Sun className="text-yellow-500 w-6 h-6" />
                            </div>
                            <h2 className="text-gray-700 font-semibold">
                                Light Level
                            </h2>
                        </div>
                        <p className="text-3xl font-bold text-gray-800 mb-2">
                            {latestData?.light_level}
                        </p>
                        <div className="flex items-center gap-2">
                            <label className="text-sm text-gray-500">
                                Threshold:
                            </label>
                            <input
                                type="number"
                                value={settings.light_threshold}
                                onChange={(e) =>
                                    updateSingleSetting(
                                        "light_threshold",
                                        parseInt(e.target.value)
                                    )
                                }
                                className="w-20 px-2 py-1 text-sm border border-gray-300 rounded-md"
                            />
                        </div>
                    </div>
                </div>

                {/* Charts */}
                <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                    <div className="bg-white p-6 rounded-xl shadow-md">
                        <h3 className="text-xl font-semibold text-gray-800 mb-6">
                            Temperature History
                        </h3>
                        <div className="h-72">
                            <ResponsiveContainer width="100%" height="100%">
                                <LineChart data={sensorData}>
                                    <CartesianGrid
                                        strokeDasharray="3 3"
                                        stroke="#f0f0f0"
                                    />
                                    <XAxis
                                        dataKey="timestamp"
                                        tickFormatter={formatDate}
                                        stroke="#6b7280"
                                    />
                                    <YAxis stroke="#6b7280" />
                                    <Tooltip content={<CustomTooltip />} />
                                    <Legend />
                                    <Line
                                        name="Temperature"
                                        type="monotone"
                                        dataKey="temperature"
                                        stroke="#EF4444"
                                        strokeWidth={2}
                                        dot={false}
                                    />
                                </LineChart>
                            </ResponsiveContainer>
                        </div>
                    </div>

                    <div className="bg-white p-6 rounded-xl shadow-md">
                        <h3 className="text-xl font-semibold text-gray-800 mb-6">
                            Soil Moisture History
                        </h3>
                        <div className="h-72">
                            <ResponsiveContainer width="100%" height="100%">
                                <LineChart data={sensorData}>
                                    <CartesianGrid
                                        strokeDasharray="3 3"
                                        stroke="#f0f0f0"
                                    />
                                    <XAxis
                                        dataKey="timestamp"
                                        tickFormatter={formatDate}
                                        stroke="#6b7280"
                                    />
                                    <YAxis stroke="#6b7280" />
                                    <Tooltip content={<CustomTooltip />} />
                                    <Legend />
                                    <Line
                                        name="Humidity"
                                        type="monotone"
                                        dataKey="humidity"
                                        stroke="#3B82F6"
                                        strokeWidth={2}
                                        dot={false}
                                    />
                                </LineChart>
                            </ResponsiveContainer>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
}
