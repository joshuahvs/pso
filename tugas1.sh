#!/bin/bash

echo "==============================================="
echo "Perangkat HANYA khusus ADMIN, semua aktifitas akan DIREKAM"
echo "Penyalahgunaan perangkat akan mendapatkan SANKSI HUKUM"
echo "==============================================="

EXPECTED_FIRST_NAME="Joshua"  # Replace with your first name
EXPECTED_NPM="2306165540"
EXPECTED_PASSWORD="Joshua"

# # Get the current date for the log file name
# CURRENT_DATE=$(date +%d%m%Y)
# LOG_FILE="pso-${CURRENT_DATE}.log"

# # Function to log activities
# log_activity() {
#     local activity=$1
#     local user_info=$2
#     local ip_address=$(hostname -I | awk '{print $1}')
#     local timestamp=$(date +"%a %d %b %Y %T %Z")
#     echo "${timestamp},${ip_address},${activity},${user_info}" >> "$LOG_FILE"
# }

main_menu() {
    while true; do
        echo "MENU"
        echo "==="
        echo
        echo "1. Informasi Status Sistem"
        echo "2. Remote Backup"
        echo "3. Berita Keamanan Kernel dari packetstomysecurity.com"
        echo "4. Keluar"
        read -p "Pilihan nomor [1-4]: " pilihan
        case $pilihan in
            1 ) menampilkan_informasi_sistem;;
            2 ) remote_backup;;
            3 ) berita_keamanan_kernel;;
            4 ) exit 0;;
            * ) echo "Pilihan tidak valid. Silakan pilih kembali.";;
        esac
    done
}


attempt=1
while [ $attempt -le 3 ]; do
    read -p "Nama Depan: " first_name
    read -p "Enter your NPM: " npm
    read -s -p "Enter your password: " password
    echo

    # Validate inputs
    if [[ "$first_name" == "$EXPECTED_FIRST_NAME" && "$npm" == "$EXPECTED_NPM" && "$password" == "$EXPECTED_PASSWORD" ]]; then
        echo "Login successful."
        # log_activity "LOGIN SUKSES" "USERPSO-${EXPECTED_NPM}"
        echo "SELAMAT DATANG ${first_name^^} - ${npm}!"
        main_menu
        break
    else
        echo "Login gagal"
        # log_activity "LOGIN GAGAL ke-${attempt}" "${first_name}-${npm}"
        attempt=$((attempt + 1))
    fi


    if [ $attempt -gt 3 ]; then
        echo "Tiga kali gagal login. Keluar aplikasi"
        # log_activity "LOGIN GAGAL 3 KALI" "${first_name}-${npm}"
        exit 1
    fi
done

menampilkan_informasi_sistem(){
    echo "Sistem ini menggunakan kernel Linux"
}

remote_backup(){
    echo "Remote Backup"
}

berita_keamanan_kernel(){
    echo "Berita Keamanan Kernel"
}