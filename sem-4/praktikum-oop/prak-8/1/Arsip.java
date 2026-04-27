public class Arsip {
	private Pesan[] daftarPesan;
	private int size;

	public Arsip() {
		this.daftarPesan = new Pesan[100];
		this.size = 0;
	}

	public void tambah(Pesan pesan) {
		if (size < daftarPesan.length) {
			daftarPesan[size] = pesan;
			size++;
		}
	}

	public Pesan get(int index) {
		return daftarPesan[index - 1];
	}

	public int cari(String keyword) {
		int count = 0;
		String keywordLower = keyword.toLowerCase();

		for (int i = 0; i < size; i++) {
			if (daftarPesan[i].getKonten().toLowerCase().contains(keywordLower)) {
				count++;
			}
		}

		return count;
	}

	public int jumlah() {
		return size;
	}
}
