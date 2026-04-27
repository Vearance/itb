public class Umandana {
    /**
     * Mengembalikan kata yang telah diubah menjadi bahasa Umandana
     * Huruf a menjadi "aiden"
     * Huruf i menjadi "ipri"
     * Huruf u menjadi "upru"
     * Huruf e menjadi "epre"
     * Huruf o menjadi "opro"
     * Huruf mati yang tidak diikuti huruf vokal menjadi huruf tersebut + "es"
     * Suku kata "ng" yang tidak diikuti huruf vokal menjadi "strengen"
     * Suku kata "ng" yang diikuti huruf vokal tetap menjadi "ng"
     * Suku kata "ny" yang diikuti huruf vokal tetap menjadi "ny"
     * Selain ketentuan di atas, huruf/karakter tidak diubah
     * *
     * 
     * @param words
     * @return kata yang telah diubah menjadi bahasa Umandana
     * 
     */
    public static String toUmandana(String words) {
        StringBuilder hasil = new StringBuilder();
        for (int i = 0; i < words.length(); i++) {
            char c = words.charAt(i);
            char lower = Character.toLowerCase(c);

            if (lower == 'n' && i + 1 < words.length()) {
                char next = Character.toLowerCase(words.charAt(i + 1));
                if (next == 'g') {
                    boolean diikutiVokal = i + 2 < words.length() && isVowel(words.charAt(i + 2));
                    if (diikutiVokal) {
                        hasil.append(words.charAt(i)).append(words.charAt(i + 1));
                    } else {
                        hasil.append("strengen");
                    }
                    i++;
                    continue;
                }
                if (next == 'y') {
                    boolean diikutiVokal = i + 2 < words.length() && isVowel(words.charAt(i + 2));
                    if (diikutiVokal) {
                        hasil.append(words.charAt(i)).append(words.charAt(i + 1));
                        i++;
                        continue;
                    }
                }
            }

            switch (lower) {
                case 'a':
                    hasil.append("aiden");
                    break;
                case 'i':
                    hasil.append("ipri");
                    break;
                case 'u':
                    hasil.append("upru");
                    break;
                case 'e':
                    hasil.append("epre");
                    break;
                case 'o':
                    hasil.append("opro");
                    break;
                default:
                    boolean hurufMati = Character.isLetter(c) && !isVowel(c);
                    boolean diikutiVokal = i + 1 < words.length() && isVowel(words.charAt(i + 1));
                    if (hurufMati && !diikutiVokal) {
                        hasil.append(c).append("es");
                    } else {
                        hasil.append(c);
                    }
                    break;
            }
        }
        return hasil.toString();
    }

    private static boolean isVowel(char c) {
        char lower = Character.toLowerCase(c);
        return lower == 'a' || lower == 'i' || lower == 'u' || lower == 'e' || lower == 'o';
    }

    /**
     * Mengembalikan kata Umandana ke bentuk normal
     * *
     * 
     * @param words kata dalam bahasa Umandana
     * @return kata telah diubah ke bentuk normal
     */
    public static String deUmandana(String words) {
        StringBuilder hasil = new StringBuilder();
        int i = 0;

        while (i < words.length()) {
            if (words.startsWith("strengen", i)) {
                hasil.append("ng");
                i += 8;
                continue;
            }

            if (words.startsWith("aiden", i)) {
                hasil.append('a');
                i += 5;
                continue;
            }
            if (words.startsWith("ipri", i)) {
                hasil.append('i');
                i += 4;
                continue;
            }
            if (words.startsWith("upru", i)) {
                hasil.append('u');
                i += 4;
                continue;
            }
            if (words.startsWith("epre", i)) {
                hasil.append('e');
                i += 4;
                continue;
            }
            if (words.startsWith("opro", i)) {
                hasil.append('o');
                i += 4;
                continue;
            }

            char c = words.charAt(i);
            boolean isHurufMati = Character.isLetter(c) && !isVowel(c);
            boolean hasEs = i + 2 < words.length() && words.charAt(i + 1) == 'e' && words.charAt(i + 2) == 's';
            if (isHurufMati && hasEs) {
                hasil.append(c);
                i += 3;
                continue;
            }

            hasil.append(c);
            i++;
        }

        return hasil.toString();
    }

}
