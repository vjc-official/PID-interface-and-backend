const { Pool } = require('pg');

// Configure the connection details
const pool = new Pool({
  user: 'your_username',
  host: 'localhost',
  database: 'my_database',
  password: 'your_password',
  port: 5432, // Default Port
});

module.exports = {
  query: (text, params) => pool.query(text, params),
};