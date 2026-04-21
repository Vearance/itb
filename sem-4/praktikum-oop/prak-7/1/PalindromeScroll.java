public class PalindromeScroll extends Scroll {
    public PalindromeScroll(String content) {
        super(content);
    }

    @Override
    protected String getType() {
        return "PALINDROME";
    }

    @Override
    public String process() {
        return new StringBuilder(getContent()).reverse().toString();
    }

    public boolean isPalindrome() {
        String normalized = getContent().replaceAll("\\s+", "").toLowerCase();
        int left = 0;
        int right = normalized.length() - 1;

        while (left < right) {
            if (normalized.charAt(left) != normalized.charAt(right)) {
                return false;
            }
            left++;
            right--;
        }

        return true;
    }
}
