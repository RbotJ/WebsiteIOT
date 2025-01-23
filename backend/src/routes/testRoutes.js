const express = require('express');
const { pool } = require('../config/db');

const router = express.Router();

// Check database connection
router.get('/db-check', async (req, res) => {
    try {
        const result = await pool.query('SELECT NOW()');
        res.json({ success: true, time: result.rows[0] });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

module.exports = router;
