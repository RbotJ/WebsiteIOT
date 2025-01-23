const { pool } = require('../config/db');

const getDevices = async (req, res) => {
    try {
        const devices = await pool.query('SELECT * FROM devices');
        res.json(devices.rows);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
};

const registerDevice = async (req, res) => {
    const { device_code } = req.body;
    try {
        const newDevice = await pool.query(
            'INSERT INTO devices (device_code, status) VALUES (?, ?)',
            [device_code, 'offline']
        );
        res.json(newDevice.rows[0]);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
};

module.exports = { getDevices, registerDevice };
