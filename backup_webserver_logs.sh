#!/usr/bin/env bash

# ==========================
# Configuration
# ==========================

# Path to the directory where your C++ web server writes logs
# Example Windows path: C:\webserver\logs
# Git Bash / MSYS2 equivalent: /c/webserver/logs
LOG_DIR="/home"

# Where backups will be stored
BACKUP_DIR="/home/backups"

# Backup file name prefix
APP_NAME="backup"

# Number of days to keep old backups (set to 0 to disable cleanup)
RETENTION_DAYS=7

# ==========================
# Script logic
# ==========================

# Exit immediately if a command fails
set -e

# Ensure log directory exists
if [[ ! -d "$LOG_DIR" ]]; then
    echo "ERROR: Log directory does not exist: $LOG_DIR"
    exit 1
fi

# Create backup directory if it does not exist
mkdir -p "$BACKUP_DIR"

# Generate timestamp
TIMESTAMP=$(date +"%Y-%m-%d_%H-%M-%S")

# Backup file name
BACKUP_FILE="${BACKUP_DIR}/${TIMESTAMP}_${APP_NAME}_.tar.gz"

# Create compressed archive
tar -czf "$BACKUP_FILE" -C "$LOG_DIR" .

echo "Backup created successfully:"
echo "  $BACKUP_FILE"

# ==========================
# Cleanup old backups
# ==========================
if [[ "$RETENTION_DAYS" -gt 0 ]]; then
    echo "Removing backups older than $RETENTION_DAYS days..."
    find "$BACKUP_DIR" -name "${APP_NAME}_logs_*.tar.gz" -type f -mtime +$RETENTION_DAYS -delete
fi

echo "Backup process completed."
