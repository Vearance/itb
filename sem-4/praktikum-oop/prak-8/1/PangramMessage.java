public class PangramMessage extends Pesan {
	public PangramMessage(String konten) {
		super(konten);
	}

	@Override
	public String process() {
		String trimmed = konten.trim();
		if (trimmed.isEmpty()) {
			return "";
		}

		String[] words = trimmed.split("\\s+");
		StringBuilder result = new StringBuilder();

		for (int i = words.length - 1; i >= 0; i--) {
			if (result.length() > 0) {
				result.append(" ");
			}
			result.append(words[i]);
		}

		return result.toString();
	}

	public boolean isPangram() {
		boolean[] found = new boolean[26];
		int uniqueCount = 0;

		String lower = konten.toLowerCase();
		for (int i = 0; i < lower.length(); i++) {
			char ch = lower.charAt(i);
			if (ch >= 'a' && ch <= 'z') {
				int index = ch - 'a';
				if (!found[index]) {
					found[index] = true;
					uniqueCount++;
				}
			}
		}

		return uniqueCount == 26;
	}

	@Override
	protected String getTipe() {
		return "PANGRAM";
	}
}
