public class KotakUtils {

    // Upper-bounded wildcard: membaca dari Kotak yang berisi subtype Barang apapun
    public static void tampilkanSemua(Kotak<? extends Barang> kotak) {
        // TODO: implementasi
        // Cetak info() setiap item, satu per baris
        // Jika kosong, cetak "Kotak kosong"
        if (kotak.kosong()) {
            System.out.println("Kotak kosong");
            return;
        }

        for (int i = 0; i < kotak.jumlah(); i++) {
            Barang item = kotak.lihat(i);
            System.out.println(item.info());
        }
    }

    // Upper-bounded wildcard: menghitung total harga
    public static int totalHarga(Kotak<? extends Barang> kotak) {
        // TODO: implementasi
        int total = 0;
        for (int i = 0; i < kotak.jumlah(); i++) {
            total += kotak.lihat(i).getHarga();
        }
        return total;
    }

    // Upper-bounded wildcard: mencari item termahal
    public static Barang termahal(Kotak<? extends Barang> kotak) {
        // TODO: implementasi
        // Kembalikan null jika kosong
        if (kotak.kosong()) {
            return null;
        }

        Barang max = kotak.lihat(0);
        for (int i = 1; i < kotak.jumlah(); i++) {
            Barang current = kotak.lihat(i);
            if (current.getHarga() > max.getHarga()) {
                max = current;
            }
        }
        return max;
    }

    // Bounded wildcard dengan type parameter (PECS: Producer Extends, Consumer Super)
    // src menggunakan ? extends T (producer/pembaca), item diambil dari src
    // dst menggunakan ? super T (consumer/penulis), item ditambahkan ke dst
    public static <T extends Barang> int pindahkan(Kotak<? extends T> src, Kotak<? super T> dst) {
        // TODO: implementasi
        // Pindahkan item dari src ke dst secara LIFO
        // Berhenti jika src kosong atau dst penuh
        // Kembalikan jumlah item yang dipindahkan
        int moved = 0;
        while (!src.kosong() && !dst.penuh()) {
            T item = src.ambil();
            if (dst.tambah(item)) {
                moved++;
            } else {
                break;
            }
        }
        return moved;
    }

    // Unbounded wildcard: hanya perlu menghitung jumlah
    public static int hitungItem(Kotak<?> kotak) {
        return kotak.jumlah();
    }
}
