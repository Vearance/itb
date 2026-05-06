import java.util.ArrayList;

public class Kotak<T extends Barang> {
    private ArrayList<T> items;
    private int kapasitas;

    public Kotak(int kapasitas) {
        this.items = new ArrayList<>();
        this.kapasitas = kapasitas;
    }

    public boolean tambah(T item) {
        if (items.size() < kapasitas) {
            items.add(item);
            return true;
        }
        return false;
    }

    public T ambil() {
        if (items.isEmpty()) {
            return null;
        }
        int lastIndex = items.size() - 1;
        T item = items.get(lastIndex);
        items.remove(lastIndex);
        return item;
    }

    public T lihat(int index) {
        if (index < 0 || index >= items.size()) {
            return null;
        }
        return items.get(index);
    }

    public int jumlah() {
        // TODO: implementasi
        return items.size();
    }

    public int kapasitas() {
        // TODO: implementasi
        return kapasitas;
    }

    public boolean penuh() {
        // TODO: implementasi
        if(items.size() >= kapasitas) return true;
        else return false;
    }

    public boolean kosong() {
        // TODO: implementasi
        return items.size()==0;
    }
}
