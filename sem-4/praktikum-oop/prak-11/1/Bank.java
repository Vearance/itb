import java.util.LinkedHashMap;
import java.util.Map;

public class Bank {
    private Map<String, BankAccount> accounts = new LinkedHashMap<>();

    /**
     * Membuat rekening baru dan menyimpannya.
     * InvalidAmountException dari konstruktor BankAccount akan ter-propagate secara otomatis
     * karena merupakan unchecked exception.
     */
    public void addAccount(String id, long initialBalance) {
        accounts.put(id, new BankAccount(id, initialBalance));
    }

    /**
     * Mengembalikan BankAccount dengan ID yang diberikan.
     * Lempar AccountNotFoundException jika tidak ditemukan.
     */
    public BankAccount getAccount(String id) throws AccountNotFoundException {
        BankAccount account = accounts.get(id);
        if (account == null) {
            throw new AccountNotFoundException(id);
        }
        return account;
    }

    /**
     * Memanggil deposit() pada rekening yang sesuai.
     * Propagate AccountNotFoundException dan exception dari BankAccount.deposit().
     */
    public void deposit(String id, long amount) throws AccountNotFoundException, AccountFrozenException {
        getAccount(id).deposit(amount);
    }

    /**
     * Memanggil withdraw() pada rekening yang sesuai.
     * Propagate AccountNotFoundException dan exception dari BankAccount.withdraw().
     */
    public void withdraw(String id, long amount)
            throws AccountNotFoundException, InsufficientFundsException, AccountFrozenException {
        getAccount(id).withdraw(amount);
    }

    /**
     * Membekukan rekening.
     * Lempar AccountNotFoundException jika tidak ditemukan.
     */
    public void freeze(String id) throws AccountNotFoundException {
        getAccount(id).freeze();
    }

    /**
     * Mengaktifkan rekening.
     * Lempar AccountNotFoundException jika tidak ditemukan.
     */
    public void unfreeze(String id) throws AccountNotFoundException {
        getAccount(id).unfreeze();
    }

    /**
     * Mentransfer amount dari rekening fromId ke toId.
     * Panggil withdraw() pada rekening asal, lalu deposit() pada rekening tujuan.
     * Propagate semua exception yang mungkin terjadi.
     */
    public void transfer(String fromId, String toId, long amount)
            throws AccountNotFoundException, InsufficientFundsException, AccountFrozenException {
        withdraw(fromId, amount);
        deposit(toId, amount);
    }
}
