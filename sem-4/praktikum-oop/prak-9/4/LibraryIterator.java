import java.util.Iterator;
import java.util.List;
import java.util.NoSuchElementException;

/**
 * Custom iterator for SmartLibrary.
 * Praktikan wajib mengimplementasikan Iterator<T> dengan method hasNext(), next(), hasPrevious(), previous().
 */
public class LibraryIterator<T> implements Iterator<T> {

    private final List<T> items;
    private int cursor; // points to the next element to return

    public LibraryIterator(List<T> items) {
        this.items = items;
        this.cursor = 0;
    }

    // ----------------------------------------------------------------
    // Implement Iterator<T>
    // ----------------------------------------------------------------

    /**
     * Mengembalikan true jika masih ada elemen tersisa.
     * cursor < size berarti list belum habis.
     */
    @Override
    public boolean hasNext() {
        return cursor < items.size();
    }

    /**
     * Mengembalikan elemen selanjutnya dan memajukan kursor.
     * Akan menghasilkan NoSuchElementException("No more elements in the library.") ketika tidak ada lagi elemen yang tersisa.
     */
    @Override
    public T next() {
        if (!hasNext()) {
            throw new NoSuchElementException("No more elements in the library.");
        }
        return items.get(cursor++);
    }

    // ----------------------------------------------------------------
    // bidirectional traversal
    // ----------------------------------------------------------------

    /**
     * Mengembalikan true jika ada elemen sebelum kursor.
     */
    public boolean hasPrevious() {
        return cursor > 0;
    }

    /**
     * Menggeser kursor mundur satu langkah dan mengembalikan elemennya.
     * Akan menghasilkan NoSuchElementException("Already at the begining of the library.") ketika tidak ada elemen sebelum cursor
     */
    public T previous() {
        if (!hasPrevious()) {
            throw new NoSuchElementException("Already at the beginning of the library.");
        }
        return items.get(--cursor);
    }

    /**
     * Mereset kursor ke awal.
     * Berguna untuk mengulang iterasi tanpa membuat iterator baru.
     */
    public void reset() {
        cursor = 0;
    }
}