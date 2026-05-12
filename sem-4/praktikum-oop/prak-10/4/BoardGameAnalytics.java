import java.util.*;
import java.util.stream.Collectors;

public class BoardGameAnalytics {

    private List<BoardGame> games;
    private Set<String> players;
    private Map<String, Integer> stockByGame;
    private Map<String, List<Integer>> ratings;

    public BoardGameAnalytics() {
        this.games = new ArrayList<>();
        this.players = new HashSet<>();
        this.stockByGame = new HashMap<>();
        this.ratings = new HashMap<>();
    }

    /**
     * Menambahkan game baru beserta stok awal.
     * Jika game dengan nama sama sudah ada, stoknya ditambah.
     *
     * @param game board game yang ditambahkan
     * @param initialStock stok awal yang ditambahkan
     */
    public void addGame(BoardGame game, int initialStock) {
        if (!stockByGame.containsKey(game.getName())) {
            games.add(game);
            stockByGame.put(game.getName(), initialStock);
        } else {
            stockByGame.put(game.getName(), stockByGame.get(game.getName()) + initialStock);
        }
    }

    /**
     * Menambahkan rating dari seorang pemain untuk sebuah game.
     * Pemain disimpan sebagai pemain unik.
     *
     * @param gameName nama game
     * @param playerName nama pemain
     * @param rating nilai rating
     */
    public void addRating(String gameName, String playerName, int rating) {
        List<Integer> daftarRating = ratings.get(gameName);
        if (daftarRating == null) {
            daftarRating = new ArrayList<>();
            ratings.put(gameName, daftarRating);
        }
        daftarRating.add(rating);
        players.add(playerName);
    }

    /**
     * Menghitung rata-rata rating sebuah game.
     * Jika belum ada rating, hasilnya 0.0.
     *
     * @param gameName nama game
     * @return rata-rata rating
     */
    private double getAverageRating(String gameName) {
        return ratings.getOrDefault(gameName, Collections.emptyList()).stream()
                .mapToInt(Integer::intValue)
                .average()
                .orElse(0.0);
    }

    /**
     * Mengembalikan daftar nama game yang stoknya kurang dari threshold.
     * Hasil diurutkan berdasarkan stok menaik.
     * Jika stok sama, urutkan berdasarkan nama game secara alfabetis.
     *
     * Method ini diharapkan menggunakan Map dan Stream API.
     *
     * @param threshold batas stok
     * @return daftar nama game dengan stok di bawah threshold
     */
    public List<String> getLowStockGames(int threshold) {
        return stockByGame.entrySet().stream()
                .filter(entry -> entry.getValue() < threshold)
                .sorted(Comparator
                        .comparingInt(Map.Entry<String, Integer>::getValue)
                        .thenComparing(Map.Entry::getKey))
                .map(Map.Entry::getKey)
                .collect(Collectors.toList());
    }

    /**
     * Mengembalikan daftar nama game yang:
     * - cocok untuk jumlah pemain tertentu
     * - memiliki rata-rata rating minimal tertentu
     * Hasil diurutkan alfabetis.
     *
     * @param playerCount jumlah pemain
     * @param minRating rating minimum
     * @return daftar nama game yang direkomendasikan
     */
    public List<String> getRecommendedGames(int playerCount, double minRating) {
        return games.stream()
                .filter(game -> game.getMinPlayers() <= playerCount && playerCount <= game.getMaxPlayers())
                .filter(game -> getAverageRating(game.getName()) >= minRating)
                .map(BoardGame::getName)
                .sorted()
                .collect(Collectors.toList());
    }

    /**
     * Main untuk mengetes implementasi secara lokal.
     *
     * Format input:
     * N
     * name minPlayers maxPlayers playTime category stock   (sebanyak N baris)
     * R
     * gameName playerName rating                           (sebanyak R baris)
     * threshold
     * playerCount minRating
     *
     * Output:
     * Baris 1:
     * LOW_STOCK <name1> <name2> ...
     * atau
     * LOW_STOCK -
     *
     * Baris 2:
     * RECOMMENDED <name1> <name2> ...
     * atau
     * RECOMMENDED -
     */
    // public static void main(String[] args) {
    //     Scanner sc = new Scanner(System.in);
    //     BoardGameAnalytics analytics = new BoardGameAnalytics();

    //     int n = sc.nextInt();
    //     for (int i = 0; i < n; i++) {
    //         String name = sc.next();
    //         int minPlayers = sc.nextInt();
    //         int maxPlayers = sc.nextInt();
    //         int playTime = sc.nextInt();
    //         String category = sc.next();
    //         int stock = sc.nextInt();

    //         analytics.addGame(
    //                 new BoardGame(name, minPlayers, maxPlayers, playTime, category),
    //                 stock
    //         );
    //     }

    //     int r = sc.nextInt();
    //     for (int i = 0; i < r; i++) {
    //         String gameName = sc.next();
    //         String playerName = sc.next();
    //         int rating = sc.nextInt();
    //         analytics.addRating(gameName, playerName, rating);
    //     }

    //     int threshold = sc.nextInt();
    //     int playerCount = sc.nextInt();
    //     double minRating = sc.nextDouble();

    //     List<String> lowStock = analytics.getLowStockGames(threshold);
    //     List<String> recommended = analytics.getRecommendedGames(playerCount, minRating);

    //     if (lowStock.isEmpty()) {
    //         System.out.println("LOW_STOCK -");
    //     } else {
    //         System.out.println("LOW_STOCK " + String.join(" ", lowStock));
    //     }

    //     if (recommended.isEmpty()) {
    //         System.out.println("RECOMMENDED -");
    //     } else {
    //         System.out.println("RECOMMENDED " + String.join(" ", recommended));
    //     }

    //     sc.close();
    // }
}