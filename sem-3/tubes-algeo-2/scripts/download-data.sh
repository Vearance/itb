#!/bin/bash

set -e

DATA_DIR="/data"
KAGGLE_DATASET_URL="https://www.kaggle.com/api/v1/datasets/download/nayakazna/project-gutenbergs-book-cover-and-content"
ZIP_FILE="/tmp/dataset.zip"

if [ -f "${DATA_DIR}/mapper.json" ] && [ -d "${DATA_DIR}/covers" ] && [ -d "${DATA_DIR}/txt" ]; then
    COVERS_COUNT=$(find "${DATA_DIR}/covers" -type f -name "*.jpg" 2>/dev/null | wc -l)
    TXT_COUNT=$(find "${DATA_DIR}/txt" -type f -name "*.txt" 2>/dev/null | wc -l)
    
    if [ "$COVERS_COUNT" -gt 0 ] && [ "$TXT_COUNT" -gt 0 ]; then
        echo "Dataset already exists with ${COVERS_COUNT} covers and ${TXT_COUNT} text files. Skipping download."
        exit 0
    fi
fi

curl -L -o "${ZIP_FILE}" "${KAGGLE_DATASET_URL}"

mkdir -p "${DATA_DIR}"

unzip -o "${ZIP_FILE}" -d "${DATA_DIR}"

if [ -d "${DATA_DIR}/project-gutenbergs-book-cover-and-content" ]; then
    echo "Moving files from subdirectory..."
    mv "${DATA_DIR}/project-gutenbergs-book-cover-and-content"/* "${DATA_DIR}/" 2>/dev/null || true
    rmdir "${DATA_DIR}/project-gutenbergs-book-cover-and-content" 2>/dev/null || true
fi

rm -f "${ZIP_FILE}"