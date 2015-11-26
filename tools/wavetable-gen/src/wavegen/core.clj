(ns wavegen.core)

(def waveforms
  {:harmonics_1 [[1 1] [1/2 3] [1/4 5] [1/8 7] [1/16 9]]
   :harmonics_2 [[1 1] [1/3 3] [-1/5 5] [1/7 7] [-1/9 9]]
   :harmonics_3 [[1 1] [1/3 2] [-1/4 4] [1/5 6] [-1/6 8] [1/7 10]]})

(defn sin-partial
  [amp f x] (* amp (Math/sin (* f x))))

(defn harmonics
  [coeffs]
  (fn [x]
    (transduce
     (map (fn [[amp f]] (sin-partial amp f x)))
     + coeffs)))

(defn generate
  ([id] (generate (name id) (harmonics (waveforms id)) 1024))
  ([id f n]
   (let [dt    (/ Math/PI (/ n 2))
         table (map #(f (* dt %)) (range n))
         norm  (/ (transduce (map #(Math/abs %)) max 0 table))
         table (map #(* norm %) table)]
     (prn :norm-scale norm)
     (str "const float " id "[" n "] = {\n"
          (->> table
               (map #(format "%1.8ff" %))
               (clojure.string/join ", "))
          "\n};"))))

;; (println (generate :harmonics_1))
