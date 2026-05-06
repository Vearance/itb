import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.DoubleSummaryStatistics;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

public class SmartLibrary<T extends Comparable<T>> implements Iterable<T>, Searchable<T> {
    private final List<T> books = new ArrayList<>();

    /**
     * Menambahkan item ke Library (list of books)
     */
    public void add(T item) {
        books.add(item);
    }

    /**
     * Menghapus item menggunakan override equals() pada T (dalam kasus ini book)
     * Returns true if itemnya ada dan berhasil diremove
     */
    public boolean remove(T item) {
        return books.remove(item);
    }

    /**
     * Jumlah buku di library (list of book)
     */
    public int size() {
        return books.size();
    }

    /**
     * Mengecek apakah ada item (Objek) ini di Library
     */
    public boolean contains(T item) {
        return books.contains(item);
    }

    /**
     * Return custom iterator untuk mengakses item di SmartLibrary.
     * Gunakan LibraryIterator
     */
    @Override
    public Iterator<T> iterator() {
        return new LibraryIterator<>(books);
    }

    // ----------------------------------------------------------------
    // Implementasi dari interface SearchAble
    // ----------------------------------------------------------------

    @Override
    public List<T> search(String keyword) {
        String needle = keyword == null ? "" : keyword.toLowerCase();
        List<T> result = new ArrayList<>();

        for (T item : books) {
            Book book = (Book) item;
            if ((book.getTitle() != null && book.getTitle().toLowerCase().contains(needle))
                    || (book.getAuthor() != null && book.getAuthor().toLowerCase().contains(needle))
                    || (book.getGenre() != null && book.getGenre().toLowerCase().contains(needle))) {
                result.add(item);
            }
        }

        return result;
    }
    
    @Override
    public boolean exists(String keyword) {
        return !search(keyword).isEmpty();
    }

    // ================================================================
    // Penggunaan Collections Algorithms
    // ================================================================

    /**
     * Melakukan pengurutan buku di library berdasarkan title (natural alphabetical order).
     */
    public void sortByTitle() {
        Collections.sort(books);
    }

    /**
     * Melakukan pengurutan buku di library berdasarkan rating (Desc: Menurun)
     */
    public void sortByRatingDesc() {
        books.sort(Comparator.comparingDouble((T item) -> ((Book) item).getRating()).reversed());
    }

    /**
     * Mencari buku berdasarkan author menggunakan binary search.
     * @return the found Book, or null if not found
     * @implNote Perhatikan pre-condition untuk binser: List harus sorted berdasarkan author!
     */
    public T findByAuthor(String author) {
        List<T> sortedByAuthor = new ArrayList<>(books);
        sortedByAuthor.sort(Comparator.comparing(
                item -> ((Book) item).getAuthor(),
                Comparator.nullsFirst(String.CASE_INSENSITIVE_ORDER)
        ));

        int left = 0;
        int right = sortedByAuthor.size() - 1;

        while (left <= right) {
            int mid = left + (right - left) / 2;
            Book book = (Book) sortedByAuthor.get(mid);
            String midAuthor = book.getAuthor();

            int cmp = Comparator.nullsFirst(String.CASE_INSENSITIVE_ORDER).compare(midAuthor, author);

            if (cmp == 0) {
                return sortedByAuthor.get(mid);
            }
            if (cmp < 0) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }

        return null; // not found
    }

    // ================================================================
    // Stream API
    // ================================================================

    /**
     * Mengembalikan top N books berdasarkan rating (descending).
     * Pipeline: stream, sorted (desc), limit, collect
     */
    public List<T> getTopRated(int n) {
        return books.stream()
                .sorted(Comparator.comparingDouble((T item) -> ((Book) item).getRating()).reversed())
                .limit(n)
                .collect(Collectors.toList());
    }

    /**
     * filter books berdasarkan genre (case-insensitive).
     * Pipeline: stream, filter, collect
     */
    public List<T> getByGenre(String genre) {
        String needle = genre == null ? "" : genre.toLowerCase();
        return books.stream()
                .filter(item -> {
                    Book book = (Book) item;
                    return book.getGenre() != null && book.getGenre().toLowerCase().equals(needle);
                })
                .collect(Collectors.toList());
    }

    /**
     * Hitung rata rata rating dari semua buku, tapi pake stream ya
     * Pipeline: stream, mapToDouble, average, orElse
     */
    public double getAverageRating() {
        return books.stream()
                .mapToDouble(item -> ((Book) item).getRating())
                .average()
                .orElse(0.0);
    }

    /**
     * Melakukan grouping semua buku berdasarkan genrenya.
     * Pipeline: stream, collect(groupingBy)
     * Akan menghasilkan map dengan key = genre string dan value = list of books dengan genre itu.
     */
    public Map<String, List<T>> groupByGenre() {
        return books.stream()
                .collect(Collectors.groupingBy(item -> ((Book) item).getGenre()));
    }

    // ================================================================
    // Statistics Report (TIDAK USAH DIIMPLEMENTASI)
    // ================================================================

    /**
     * Bonus : melakukan print statistik per-genre menggunakan summarizingDouble. DoubleSummaryStatistics holds count, min, max, sum, average all at once.
     *
     * Pipeline: stream, collect(groupingBy + summarizingDouble), forEach print
     */
    public void printGenreReport() {
        Map<String, DoubleSummaryStatistics> stats = books.stream()
                .collect(Collectors.groupingBy(
                        t -> ((Book) t).getGenre(),
                        Collectors.summarizingDouble(t -> ((Book) t).getRating())
                ));

        System.out.println("\n===== Genre Rating Report =====");
        stats.entrySet().stream()
                .sorted(Map.Entry.comparingByKey())
                .forEach(entry -> {
                    DoubleSummaryStatistics s = entry.getValue();
                    System.out.printf("%-15s | count=%-3d | min=%.1f | max=%.1f | avg=%.2f%n",
                            entry.getKey(), s.getCount(), s.getMin(), s.getMax(), s.getAverage());
                });
        System.out.println("================================\n");
    }
}