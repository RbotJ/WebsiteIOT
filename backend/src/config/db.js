//db.js
const mysql = require('mysql2/promise');
const fs = require('fs');
require('dotenv').config();

// Use MYSQL_URL if available, fallback to DATABASE_URL
const databaseUrl = process.env.MYSQL_URL || process.env.DATABASE_URL;

const parseDatabaseUrl = (url) => {
    try {
        const { hostname, port, pathname, username, password } = new URL(url);
        return {
            host: hostname,
            port: port || 3306,
            user: username || 'root',
            password: password || '',
            database: pathname.replace('/', ''), // Remove leading "/"
            waitForConnections: true,
            connectionLimit: 10,
            queueLimit: 0,
            ssl: process.env.NODE_ENV === "production" ? { rejectUnauthorized: false } : false // Enable SSL in production
        };
    } catch (err) {
        console.error("❌ Invalid DATABASE_URL format:", err.message);
        process.exit(1);
    }
};

// Create MySQL connection pool
const pool = mysql.createPool(parseDatabaseUrl(databaseUrl));

const connectDB = async (retries = 5) => {
    while (retries) {
        let conn;
        try {
            conn = await pool.getConnection();
            const [rows] = await conn.query('SELECT NOW() AS now');
            console.log(`✅ MySQL Connected: ${rows[0].now}`);
            conn.release();
            await runMigrations(); // Ensure tables exist
            return;
        } catch (err) {
            console.error(`❌ Database connection failed: ${err.message}`);
            retries -= 1;
            if (conn) conn.release(); // Ensure connection is released
            if (retries > 0) {
                console.log(`🔄 Retrying... (${5 - retries}/5) in 5 seconds`);
                await new Promise(res => setTimeout(res, 5000)); // Wait 5 sec before retrying
            } else {
                console.error("❌ Unable to connect to MySQL after multiple attempts. Exiting.");
                process.exit(1);
            }
        }
    }
};

// Auto-run `init.sql` if present
const runMigrations = async () => {
    try {
        if (fs.existsSync('./scripts/init.sql')) {
            const sql = fs.readFileSync('./scripts/init.sql', 'utf8');
            await pool.query(sql);
            console.log('✅ Database initialized successfully');
        } else {
            console.warn('⚠️ No `init.sql` found. Skipping database initialization.');
        }
    } catch (err) {
        console.error('❌ Database initialization error:', err.message);
    }
};

// Log parsed connection details (excluding password)
console.log(`🔍 Connecting to MySQL @ ${parseDatabaseUrl(databaseUrl).host}:${parseDatabaseUrl(databaseUrl).port}`);

connectDB();

module.exports = { pool, connectDB };
