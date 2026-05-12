public class LaciUtil {
    public static <T> void tukar(Laci<T> laci, int i, int j) {
        if (i < 1 || j < 1 || i > laci.ukuran() || j > laci.ukuran() || i == j) {
            return;
        }
        Object temp = laci.arr[i - 1];
        laci.arr[i - 1] = laci.arr[j - 1];
        laci.arr[j - 1] = temp;
    }

    public static <T extends Comparable<T>> T terbesar(Laci<T> laci) {
        if (laci.ukuran() == 0) {
            return null;
        }

        T max = (T) laci.arr[0];
        for (int i = 1; i < laci.ukuran(); i++) {
            T current = (T) laci.arr[i];
            if (current.compareTo(max) > 0) {
                max = current;
            }
        }
        return max;
    }

}
