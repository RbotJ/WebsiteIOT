const { Pool } = require('pg');
require('dotenv').config();

const pool = new Pool({
    connectionString: process.env.DATABASE_URL,
    ssl: {
        rejectUnauthorized: false
    }
});

const connectDB = async () => {
    try {
        await pool.query('SELECT NOW()');
        console.log('PostgreSQL Connected');
    } catch (err) {
        console.error(err.message);
        process.exit(1);
    }
};

module.exports = { pool, connectDB };
