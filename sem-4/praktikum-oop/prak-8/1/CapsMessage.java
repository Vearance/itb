public class CapsMessage extends Pesan {
	public CapsMessage(String konten) {
		super(konten);
	}

	@Override
	public String process() {
		return konten.toUpperCase();
	}

	public int countVowels() {
		int count = 0;
		String lower = konten.toLowerCase();
		for (int i = 0; i < lower.length(); i++) {
			char ch = lower.charAt(i);
			if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u') {
				count++;
			}
		}
		return count;
	}

	@Override
	protected String getTipe() {
		return "CAPS";
	}
}
