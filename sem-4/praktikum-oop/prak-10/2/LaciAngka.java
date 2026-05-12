public class LaciAngka<T extends Number> extends Laci<T> {
    public LaciAngka(String label) {
        super(label);
    }

    public double total() {
        double jumlah = 0.0;
        for (int i = 0; i < cap; i++) {
            if (arr[i] != null) {
                jumlah += ((Number) arr[i]).doubleValue();
            }
        }
        return jumlah;
    }

    public double rataRata() {
        if (ukuran() == 0) {
            return 0.0;
        }
        return total() / ukuran();
    }
}