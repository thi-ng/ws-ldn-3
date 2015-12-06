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

(defn sin-pow
  [coeffs]
  (fn [x]
    (transduce
     (map (fn [[amp f e]] (* amp (Math/pow (Math/sin (* f x)) e))))
     + coeffs)))

(defn sin-pow2
  []
  (let [base (sin-pow [[1 1 7] [1/2 3 8] [1/4 5 9]])]
    (fn [x]
      (* 0.78 (- (base x) 0.5)))))

(defn sin-exp
  []
  (fn [x]
    (* (- (Math/sin (Math/exp (- (* 1.5 x) 4)))
          (Math/sin (Math/exp (+ (* 0.5 x) 2))))
       (Math/sin (* 0.5 x)) (/ 1.0 1.92))))

(defn saw [amp f x] (* amp (dec (/ (mod (* f x) (* 2 Math/PI)) Math/PI))))

(defn super-saw
  [coeffs amp]
  (fn [x]
    (* amp
       (transduce
        (map (fn [[f a fs]] (f a fs x)))
        + coeffs))))

(defn sin-exp2
  []
  (fn [x]
    (+ (* 0.5 (Math/sin x))
       (- (* 0.26 (Math/exp (mod x (/ Math/PI 2)))) 0.75))))

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

;; (println (generate 'sin_pow (sin-pow [[1 1 5] [-1/4 4 12]]) 1024))
;; (println (generate 'sin_exp (sin-pow2) 1024))
;; (println (generate 'sin_exp (sin-exp) 1024))
;; (println (generate 'super_saw (super-saw [[saw 1 1] [saw -1/2 2] [saw 1/4 3] [sin-partial 1/6 4]] (/ 1 0.78)) 1024))
