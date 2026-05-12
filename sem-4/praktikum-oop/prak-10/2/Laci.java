public class Laci<T> {
    protected Object[] arr;
    protected int cap = 0;
    protected String label;

    @SuppressWarnings("unchecked")
    public Laci(String label) {
        this.arr = new Object[10];
        this.label = label;
    }

    public boolean simpan(T item) {
        if (cap >= 10) {
            return false;
        }
        arr[cap] = item;
        cap++;
        return true;
    }

    public T ambil(int i) {
        if (i < 1 || i > cap) {
            return null;
        }
        return (T) arr[i - 1];
    }

    public void set(int i, T item) {
        if (i < 1 || i > cap) {
            return;
        }
        arr[i - 1] = item;
    }

    public int ukuran() {
        return cap;
    }

    public String getLabel() {
        return label;
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("Laci[").append(label).append("]: [");
        for (int i = 0; i < cap; i++) {
            if (i > 0) {
                sb.append(", ");
            }
            sb.append(arr[i]);
        }
        sb.append("]");
        return sb.toString();
    }
}