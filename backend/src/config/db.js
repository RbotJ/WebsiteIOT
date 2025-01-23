const { Pool } = require('pg');
require('dotenv').config();

// Configure SSL only for production environments
const isProduction = process.env.NODE_ENV === 'production';

const pool = new Pool({
    connectionString: process.env.DATABASE_URL,
    ssl: isProduction ? { rejectUnauthorized: false } : false
});

const connectDB = async () => {
    try {
        const res = await pool.query('SELECT NOW()');
        console.log(`✅ PostgreSQL Connected: ${res.rows[0].now}`);
    } catch (err) {
        console.error('❌ Database connection error:', err.message);
        process.exit(1);
    }
};

// Log connection status when module is loaded
connectDB();

module.exports = { pool, connectDB };
