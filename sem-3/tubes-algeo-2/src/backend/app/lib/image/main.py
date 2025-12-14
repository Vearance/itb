import logging
import numpy as np
import pathlib as path
from PIL import Image
import json

from app.lib.maths import svd

logger = logging.getLogger("uvicorn.error")


class ImageProcessor:
    def __init__(self, cover_folder: str, mapper_file: str, cache_dir: str, cover_output_folder: str = ""):
        self.images_count = 0
        self.image_map = dict()

        self.eigencovers: np.ndarray = np.array([])
        self.avg_vector: np.ndarray = np.array([])
        self.image_coefficients_normalized: np.ndarray = np.array([])

        if mapper_file:
            self._load_mapper(mapper_file)
        else:
            raise ValueError("Mapper file must be provided.")

        cache_files = ["eigencovers.npz", "avg_vector.npz",
                       "image_coefficients_normalized.npz", "image_map.json"]
        if cache_dir and path.Path(cache_dir).exists() and all((path.Path(cache_dir) / f).exists() for f in cache_files):
            self._load_from_cache(cache_dir)
        else:
            if cover_folder:
                if cover_output_folder == "":
                    cover_output_folder = cover_folder + "_grayscale"
                self._prepare_and_process_images(
                    cover_folder, cover_output_folder)
                if cache_dir:
                    self._save_to_cache(cache_dir)
            else:
                raise ValueError("Cover folder must be provided.")

    def _load_mapper(self, mapper_file: str):
        if not path.Path(mapper_file).exists():
            raise FileNotFoundError(
                f"Mapper file {mapper_file} does not exist.")

        with open(mapper_file, 'r') as f:
            mapper_data = json.load(f)

        for image_id, metadata in mapper_data.items():
            cover_filename = path.Path(metadata.get("cover", "")).name
            if cover_filename:
                self.image_map[cover_filename] = {
                    "id": image_id,
                    "title": metadata.get("title", "Unknown"),
                }

    def _save_to_cache(self, cache_dir: str):
        np.savez_compressed(
            path.Path(cache_dir) / "eigencovers.npz",
            eigencovers=self.eigencovers
        )
        np.savez_compressed(
            path.Path(cache_dir) / "avg_vector.npz",
            avg_vector=self.avg_vector
        )
        np.savez_compressed(
            path.Path(cache_dir) / "image_coefficients_normalized.npz",
            image_coefficients_normalized=self.image_coefficients_normalized
        )
        # Save image_map with numeric indices
        with open(path.Path(cache_dir) / "image_map.json", 'w') as f:
            # Convert keys to strings for JSON compatibility
            json.dump({str(k): v for k, v in self.image_map.items() if isinstance(k, int)}, f)
        logger.debug("Image processor data saved to cache.")

    def _load_from_cache(self, cache_dir: str):
        eigencovers_path = path.Path(cache_dir) / "eigencovers.npz"
        avg_vector_path = path.Path(cache_dir) / "avg_vector.npz"
        image_coefficients_normalized_path = path.Path(
            cache_dir) / "image_coefficients_normalized.npz"
        image_map_path = path.Path(cache_dir) / "image_map.json"

        if not eigencovers_path.exists() or not avg_vector_path.exists() or not image_coefficients_normalized_path.exists():
            raise FileNotFoundError(
                "Cached files not found in the specified cache directory.")

        self.eigencovers = np.load(eigencovers_path, allow_pickle=True)['eigencovers']
        self.avg_vector = np.load(avg_vector_path, allow_pickle=True)['avg_vector']
        self.image_coefficients_normalized = np.load(
            image_coefficients_normalized_path, allow_pickle=True)['image_coefficients_normalized']
        

        # Build a lookup from id to title from mapper data
        id_to_title = {}
        for filename, data in self.image_map.items():
            if isinstance(filename, str):
                id_to_title[data["id"]] = data.get("title", "Unknown")

        # Load image_map from cache
        if image_map_path.exists():
            with open(image_map_path, 'r') as f:
                cached_map = json.load(f)
                # Convert string keys back to integers and ensure title exists
                for k, v in cached_map.items():
                    # If title is missing, try to get it from mapper data
                    if "title" not in v:
                        v["title"] = id_to_title.get(v["id"], "Unknown")
                    self.image_map[int(k)] = v

        logger.debug("Image processor data loaded from cache.")

    def _prepare_and_process_images(self, input_dir: str, output_dir: str):
        if not path.Path(input_dir).exists():
            raise FileNotFoundError(
                f"Input directory {input_dir} does not exist.")

        if not path.Path(output_dir).exists():
            path.Path(output_dir).mkdir(parents=True)

        img_files = list(path.Path(input_dir).glob("*.jpg"))
        if not img_files:
            raise ValueError(f"No .jpg images found in {input_dir}")

        first_img = Image.open(img_files[0]).convert("RGB")
        img_shape = np.array(first_img).shape[:2]
        vector_size = img_shape[0] * img_shape[1]
        del first_img

        self.images_count = len(img_files)

        image_vectors = np.zeros(
            (self.images_count, vector_size), dtype=np.float32)

        grayscale_weights = np.array(
            [0.2126, 0.7152, 0.0722], dtype=np.float32)

        logger.debug(f"Processing {self.images_count} images...")
        for idx, img_file in enumerate(img_files):
            img = Image.open(img_file).convert("RGB")
            img_array = np.array(img, dtype=np.float32)

            gray_img = np.dot(img_array[..., :3], grayscale_weights)

            output_path = path.Path(output_dir) / img_file.name
            Image.fromarray(gray_img.astype(np.uint8)).save(output_path)

            image_vectors[idx] = gray_img.flatten() / 255.0

            if img_file.name in self.image_map:
                self.image_map[idx] = self.image_map[img_file.name]
            else:
                self.image_map[idx] = {"id": img_file.name, "title": "Unknown"}

        self.avg_vector = np.mean(image_vectors, axis=0)
        image_vectors -= self.avg_vector

        logger.debug("Decomposing image vectors using SVD...")
        U, _, _ = svd(image_vectors.T)

        self.eigencovers = U.astype(np.float32)

        all_coefficients = np.dot(self.eigencovers.T, image_vectors.T)

        norms = np.linalg.norm(all_coefficients, axis=0, keepdims=True)
        norms[norms < 1e-10] = 1.0
        self.image_coefficients_normalized = (
            all_coefficients / norms).T.astype(np.float32)

        del image_vectors

    def search_cover_by_image(self, query_image_path: str, top_k: int = 5):
        if not path.Path(query_image_path).exists():
            raise FileNotFoundError(
                f"Query image {query_image_path} does not exist.")

        img = Image.open(query_image_path).convert("RGB")
        img_array = np.array(img, dtype=np.float32)
        gray_img = np.dot(img_array[..., :3], [0.2126, 0.7152, 0.0722]) / 255.0

        query_vector = gray_img.flatten() - self.avg_vector

        query_coefficients = np.dot(self.eigencovers.T, query_vector)

        query_norm = np.linalg.norm(query_coefficients)
        if query_norm < 1e-10:
            query_coefficients_normalized = query_coefficients
        else:
            query_coefficients_normalized = query_coefficients / query_norm

        scores = np.dot(self.image_coefficients_normalized,
                        query_coefficients_normalized)

        if top_k < self.images_count:
            top_indices = np.argpartition(scores, -top_k)[-top_k:]
            top_indices = top_indices[np.argsort(scores[top_indices])[::-1]]
        else:
            top_indices = np.argsort(scores)[::-1][:top_k]

        results = []
        for idx in top_indices:
            image_info = self.image_map.get(int(idx), {"id": "Unknown", "title": "Unknown"})
            results.append({
                "id": image_info["id"],
                "title": image_info.get("title", "Unknown"),
                "similarity_score": float(scores[idx])
            })

        return results
