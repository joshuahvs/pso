#!/bin/bash

# NAMA: Joshua Hans Vito Soehendra
# NPM: 2306165540
# LINK VIDEO DEMO: https://youtu.be/Ty5yOjVfn9w

echo "==========================================================="
echo "Perangkat HANYA khusus ADMIN, semua aktifitas akan DIREKAM"
echo "Penyalahgunaan perangkat akan mendapatkan SANKSI HUKUM"
echo "==========================================================="

EXPECTED_FIRST_NAME="Joshua"
EXPECTED_NPM="2306165540"
EXPECTED_PASSWORD="Joshua"

TANGGAL=$(date '+%a %d %b %Y %H:%M:%S %Z')
IP_ADDRESS=$(hostname -I | awk '{print $1}')
# Fungsi untuk log 
log_activity() {        
    # Menentukan nama file log dengan format pso-tanggal.log
    nama_file_log="pso-$(date '+%d%m%Y').log"
    # Membuat direktori log jika belum ada
    echo "$TANGGAL,$ip_address, $1" >> "$nama_file_log"
}

# Fungsi untuk mengirimkan pesan ke whatsapp menggunakan twillio
send_message(){
    local message=$(printf "$1")
    curl 'https://api.twilio.com/2010-04-01/Accounts/AC3f7f5caf217fec848d5561afa79a4dd9/Messages.json' -X POST \
    --data-urlencode 'To=whatsapp:+628121697249' \
    --data-urlencode 'From=whatsapp:+14155238886' \
    --data-urlencode "Body=$message" \
    -u AC3f7f5caf217fec848d5561afa79a4dd9:630825331b7248b63f580903fa2c52fa > /dev/null 2>&1
}


menampilkan_informasi_sistem(){
    while true; do
        echo "Status Sistem"
        echo "-------------"

        # Utilisasi CPU
        cpu_util=$(mpstat | awk '/all/ { total_util = $3 + $5; print ""total_util"%" }')
        echo "Utilisasi CPU: $cpu_util"

        # Penggunaan Memori
        read total used free shared buff_cache available <<< $(free -m | awk '/Mem:/ {print $2, $3, $4, $5, $6, $7}') #CEK LAGI JUGA YANG INI
        mem_percent=$(awk -v used="$used" -v total="$total" 'BEGIN {printf("%.2f", (used/total)*100)}')
        echo "Penggunaan Memori = $used/$total MBytes ($mem_percent%)"

        # Penggunaan Disk
        disk_info=$(df -h / | awk 'NR==2 {print $3, $2, $5}')
        disk_used=$(echo $disk_info | awk '{print $1}')
        disk_total=$(echo $disk_info | awk '{print $2}')
        disk_percent=$(echo $disk_info | awk '{print $3}')
        echo "Penggunaan Disk = $disk_used/$disk_total ($disk_percent)"

        echo ""

        read -p "Kembali ke menu? [y/n]: " pilihan
        case $pilihan in
            y ) break;;
            n ) exit 0;;
            * ) echo "Pilihan tidak valid. Silakan pilih kembali.";;
        esac
    done
    main_menu
}

# Fungsi untuk melakukan push
push(){
    while true; do
        # Meminta informasi tujuan
        read -p "IP Server Target: " target_ip
        read -p "Akun pada target server: " target_user
        read -p "Path folder sumber: " src_path

        # Melakukan push
        rsync_output=$(rsync -avh --delete "$src_path/" "$target_user@$target_ip:$src_path")
        rsync_exit_status=$?
        echo "$rsync_output"

        # Mengecheck status push dan mengirim pesan ke whatsapp
        if [ $rsync_exit_status -eq 0 ]; then
            echo
            echo "Status backup BERHASIL telah diinformasikan ke Admin sistem"
            send_message "$(printf "Notifikasi Backup:*BERHASIL*\nWaktu : $TANGGAL\nOperasi: rsync -avh --delete $target_user@$target_ip:$src_path/\nMetode: Push\nUsername : $first_name\nNPM : $npm\nIP : $IP_ADDRESS")"
        else
            echo
            echo "Status backup GAGAL telah diinformasikan ke Admin sistem"
            send_message "$(printf "Notifikasi Backup:*GAGAL*\nWaktu : $TANGGAL\nOperasi: rsync -avh --delete $target_user@$target_ip:$src_path/\nMetode: Push\nUsername : $first_name\nNPM : $npm\nIP : $IP_ADDRESS")"
        fi
        # Menambahkan ke log
        log_activity "Remote Backup - Push,rsync -avh --delete $src_path/ $target_user@$target_ip:$src_path, ${first_name}-${npm}"

        echo
        # Pilihan untuk kembali ke menu
        read -p "Kembali ke menu? [y/n]: " pilihan
        case $pilihan in
            y ) break;;
            n ) exit 0;;
            * ) echo "Pilihan tidak valid. Kembali ke menu utama.";;
        esac
    done
    main_menu
}

# Fungsi untuk melakukan pull 
pull(){
    while true; do
        # Meminta informasi sumber
        read -p "IP Server Sumber: " source_ip
        read -p "Akun pada server sumber: " source_user
        read -p "Path folder sumber: " src_path

        # Melakukan pull
        rsync_output=$(rsync -avh --delete "$source_user@$source_ip:$src_path/" "$src_path" 2>&1)
        rsync_exit_status=$?
        echo "$rsync_output"

        # Mengecheck status pull dan mengirim pesan ke whatsapp
        if [ $rsync_exit_status -eq 0 ]; then
            echo
            echo "Status backup BERHASIL telah diinformasikan ke Admin sistem"
            send_message "$(printf "Notifikasi Backup:*BERHASIL*\nWaktu : $TANGGAL\nOperasi: rsync -avh --delete $target_user@$target_ip:$src_path/\nMetode: Pull\nUsername : $first_name\nNPM : $npm\nIP : $IP_ADDRESS")"
        else
            echo
            echo "Status backup GAGAL telah diinformasikan ke Admin sistem"
            send_message "$(printf "Notifikasi Backup:*GAGAL*\nWaktu : $TANGGAL\nOperasi: rsync -avh --delete $target_user@$target_ip:$src_path/\nMetode: Pull\nUsername : $first_name\nNPM : $npm\nIP : $IP_ADDRESS")"
        fi

        # Menambahkan ke log
        log_activity "Remote Backup - Pull,rsync -avh --delete $target_user@$target_ip:$src_path/ $src_path, ${first_name}-${npm}"

        # Pilihan untuk kembali ke menu
        read -p "Kembali ke menu? [y/n]: " pilihan
        case $pilihan in
            y ) break;;
            n ) exit 0;;
            * ) echo "Pilihan tidak valid. Kembali ke menu utama.";;
        esac
    done
    main_menu

}

# Fungsi untuk menampilkan pilihan antara mau push, pull, atau back
remote_backup(){
    while true; do
        read -p "Push atau Pull backup [push/pull/back]? " pilihan_method
        case $pilihan_method in
                push ) push;;
                pull ) pull;;
                back ) break;;
                * ) echo "Pilihan tidak valid. Silakan pilih kembali.";;
        esac
    done
    main_menu
}

berita_keamanan_kernel(){
    echo "BERITA KERNEL TERBARU"
    echo "====================="

    # Mengambil konten RSS feed dari packetstormsecurity
    rss_content=$(curl -s "https://rss.packetstormsecurity.com/files/tags/kernel/")

    # File untuk output XML
    output_file="kernel.xml"

    # Menyimpan konten RSS ke file XML
    echo "$rss_content" > "$output_file"
    echo "Konten berita kernel telah disimpan ke $output_file"

    # Menampilkan konten dengan format yang lebih mudah dibaca
    echo "$rss_content" | xmlstarlet sel -t -m "//item[position()<=5]" \
        -v "concat('Tanggal: ', pubDate)" -n \
        -v "concat('0',position(), '. ', description)" -n -n

    echo ""
    read -p "Kembali ke menu? [y/n]: " pilihan
    case $pilihan in
        y|Y ) main_menu;;
        n|N ) log_activity "LOGOUT"; exit 0;;
        * ) echo "Pilihan tidak valid. Kembali ke menu utama."; main_menu;;
    esac
}


# Fungsi untuk menampilkan pilihan menu
main_menu() {
    # looping agar tidak keluar sebelum memilih 4
    while true; do
        # menampilkan menu
        echo "MENU"
        echo "==="
        echo
        echo "1. Informasi Status Sistem"
        echo "2. Remote Backup"
        echo "3. Berita Keamanan Kernel dari packetstomysecurity.com"
        echo "4. Keluar"
        read -p "Pilihan nomor [1-4]: " pilihan
        echo
        # melakukan fungsi berdasarkan pilihan
        case $pilihan in
            1 ) menampilkan_informasi_sistem;;
            2 ) remote_backup;;
            3 ) berita_keamanan_kernel;;
            4 ) exit 0;;
            * ) echo "Pilihan tidak valid. Silakan pilih kembali.";;
        esac
    done
}

# Melakukan validasi login 3 kali
attempt=1
while [ $attempt -le 3 ]; do
    # Meminta input
    read -p "Nama Depan: " first_name
    read -p "Enter your NPM: " npm
    read -s -p "Enter your password: " password

    # Memvalidasi input
    if [[ "$first_name" == "$EXPECTED_FIRST_NAME" && "$npm" == "$EXPECTED_NPM" && "$password" == "$EXPECTED_PASSWORD" ]]; then
        # Login berhasil, menyimpan log dan menampilkan menu
        log_activity "LOGIN SUKSES, ${first_name^^}-${npm}"
        echo "SELAMAT DATANG ${first_name^^} - ${npm}!"
        main_menu
        break
    else
        echo
        echo "Login gagal"
        echo
        #Jika sudah tiga kali gagal atau input salah
        if [ $attempt -eq 3 ]; then
            echo "Tiga kali gagal login. Keluar aplikasi"
            echo
            echo "Insiden ini telah dilaporkan ke Admin sistem."
            log_activity "LOGIN GAGAL 3 KALI, ${first_name}-${npm}"
            send_message "$(printf "*PERINGATAN KEAMANAN*: Tiga kali Gagal Login Tugas 1-PSO:\nWaktu : $TANGGAL\nUsername : $first_name\nNPM : $npm\nIP : $IP_ADDRESS\nHarap segera ditindak lanjuti!")"
            exit 1
        fi
        # Jika belum tiga kali gagal, menambah attempt
        log_activity "LOGIN GAGAL ke-$attempt, ${first_name^^}-${npm}"
        attempt=$((attempt + 1))
    fi
done