import java.util.LinkedHashMap;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;

public class Storage<T> {
    private Map<String, T> data;
    private int capacity;

    public Storage(int capacity) {
        this.capacity = capacity;
        this.data = new LinkedHashMap<>();
    }

    public void store(String id, T item) throws StorageFullException, DuplicateIdException {
        // TODO:
        // Tambahkan item ke dalam data dengan id sebagai key.

        // Jika kapasitas sudah penuh, lemparkan StorageFullException.
        if(capacity <= data.size()) {
            throw new StorageFullException();
        }

        // Jika kapasitas belum penuh namun id sudah ada di dalam data, lemparkan DuplicateIdException.
        if (capacity > data.size() && data.containsKey(id)) {
            throw new DuplicateIdException(id);
        }

        data.put(id, item);
    }

    public T retrieve(String id)throws DataNotFoundException {
        // TODO:
        // Kembalikan item yang sesuai dengan id.

        // Jika id tidak ditemukan, lemparkan DataNotFoundException.
        boolean found = data.containsKey(id);

        if (!found) {
            throw new DataNotFoundException(id);
        }
        else { // found
            return data.get(id);
        }
    }
    
    public void remove(String id) throws DataNotFoundException {
        // TODO:
        // Hapus item dengan id dari dalam data.
        // Jika id tidak ditemukan, lemparkan DataNotFoundException.
        boolean found = data.containsKey(id);

        if (!found) {
            throw new DataNotFoundException(id);
        }
        else { // found
            data.remove(id);
        }
    }


    // private Map<String, T> data;
    // private int capacity;

    // public Storage(int capacity) {
    //     this.capacity = capacity;
    //     this.data = new LinkedHashMap<>();
    // }

    public List<T> getAll() {
        // TODO:
        // Kembalikan semua item di dalam data dalam bentuk List.
        return new ArrayList<>(data.values());

    }
    
    public int getCapacity() {
        return this.capacity;
    }
    
    public int getSize() {
        if (this.data == null) {
            return 0;
        }
        return this.data.size();
    }
}
