public class RepeatMessage extends Pesan {
	private int n;

	public RepeatMessage(String konten, int n) {
		super(konten);
		this.n = n;
	}

	public int getN() {
		return n;
	}

	@Override
	public String process() {
		String trimmed = konten.trim();
		if (trimmed.isEmpty()) {
			return "";
		}

		String[] words = trimmed.split("\\s+");
		StringBuilder result = new StringBuilder();

		for (String word : words) {
			for (int i = 0; i < n; i++) {
				if (result.length() > 0) {
					result.append(" ");
				}
				result.append(word);
			}
		}

		return result.toString();
	}

	@Override
	protected String getTipe() {
		return "REPEAT";
	}
}
