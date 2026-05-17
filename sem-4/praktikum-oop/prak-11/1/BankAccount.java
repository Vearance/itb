public class BankAccount {
    private String id;
    private long balance;
    private boolean frozen;

    /**
     * Membuat rekening baru.
     * Lempar InvalidAmountException jika initialBalance < 0.
     */
    public BankAccount(String id, long initialBalance) {
        if (initialBalance < 0) {
            throw new InvalidAmountException(initialBalance);
        }
        else{
            this.id = id;
            this.balance = initialBalance;
            this.frozen = false;
        }
    }

    /** Mengembalikan ID rekening. */
    public String getId() {
        return this.id;
    }

    /** Mengembalikan saldo saat ini. */
    public long getBalance() {
        return this.balance;
    }

    /** Mengembalikan true jika rekening sedang dibekukan. */
    public boolean isFrozen() {
        if(this.frozen) return true;
        else return false;
    }

    /**
     * Menambah saldo sebesar amount.
     * Urutan pengecekan:
     *   1. amount <= 0  -> InvalidAmountException
     *   2. frozen       -> AccountFrozenException
     */
    public void deposit(long amount) throws InvalidAmountException, AccountFrozenException {
        if (amount <= 0) {
            throw new InvalidAmountException(amount);
        }
        if (frozen) {
            throw new AccountFrozenException(id);
        }
        balance += amount;
    }

    /**
     * Mengurangi saldo sebesar amount.
     * Urutan pengecekan:
     *   1. amount <= 0        -> InvalidAmountException
     *   2. frozen             -> AccountFrozenException
     *   3. amount > balance   -> InsufficientFundsException
     */
    public void withdraw(long amount) throws InvalidAmountException, InsufficientFundsException, AccountFrozenException {
        if (amount <= 0) {
            throw new InvalidAmountException(amount);
        }
        if (frozen) {
            throw new AccountFrozenException(id);
        }
        if (amount > balance) {
            throw new InsufficientFundsException(id, balance, amount);
        }
        balance -= amount;
    }

    /** Membekukan rekening. */
    public void freeze() {
        frozen = true;
    }

    /** Mengaktifkan kembali rekening yang dibekukan. */
    public void unfreeze() {
        frozen = false;
    }
}
