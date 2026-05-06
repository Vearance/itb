package main

import (
	"backend/models"
	"backend/services"
	"backend/services/parser"
	"backend/services/search"
	"log"

	"github.com/go-playground/validator/v10"
	"github.com/gofiber/fiber/v3"
	"github.com/gofiber/fiber/v3/middleware/cors"
)

type structValidator struct {
	validate *validator.Validate
}

func (v *structValidator) Validate(out any) error {
	return v.validate.Struct(out)
}

func main() {
	app := fiber.New(fiber.Config{
		StructValidator: &structValidator{
			validate: validator.New(),
		},
	})

	app.Use(cors.New(cors.Config{
		AllowOrigins: []string{"*"},
		AllowMethods: []string{"GET", "POST", "OPTIONS"},
		AllowHeaders: []string{"Origin", "Content-Type", "Accept"},
	}))

	app.Post("/", func(c fiber.Ctx) error {
		if !c.HasBody() {
			return c.SendStatus(fiber.StatusBadRequest)
		}

		req := new(models.Request)

		if err := c.Bind().Body(req); err != nil {
			if validationErrors, ok := err.(validator.ValidationErrors); ok {
				for _, fieldErr := range validationErrors {
					return c.Status(fiber.StatusBadRequest).JSON(fiber.Map{
						"field": fieldErr.Field(),
						"error": fieldErr.Error(),
					})
				}
			} else {
				log.Printf("Error binding request body: %v\n", err)
				return c.SendStatus(fiber.StatusBadRequest)
			}
			return err
		}

		rawHTML, err := services.FetchHTMLPage(req.URL)

		if err != nil {
			log.Printf("Error fetching HTML page: %v\n", err)
			return c.SendStatus(fiber.StatusInternalServerError)
		}

		DOMTree, err := parser.ParseHTML(rawHTML)

		if err != nil {
			log.Printf("Error parsing HTML: %v\n", err)
			return c.SendStatus(fiber.StatusInternalServerError)
		}

		if req.Selector == "" {
			log.Println("No selector provided for DFS search")
			return c.SendStatus(fiber.StatusBadRequest)
		}

		sel, err := parser.ParseCSSSelector(req.Selector)

		if err != nil {
			log.Printf("Error parsing CSS selector: %v\n", err)
			return c.SendStatus(fiber.StatusBadRequest)
		}

		if req.Type == "DFS" {
			log.Printf("DFS request received: URL=%s, Amount=%d, Selector=%s\n", req.URL, req.Amount, req.Selector)

			res, searchlog := search.SearchElementDFS(DOMTree, &sel, req.Amount)

			return c.JSON(map[string]interface{}{
				"DOMTree": DOMTree.Serialize(),
				"result":  res.Serialize(),
				"log":     searchlog.Serialize(),
			})

		} else {
			log.Printf("BFS request received: URL=%s, Amount=%d, Selector=%s\n", req.URL, req.Amount, req.Selector)

			res, searchlog := search.SearchElementBFS(DOMTree, &sel, req.Amount)

			return c.JSON(map[string]interface{}{
				"DOMTree": DOMTree.Serialize(),
				"result":  res.Serialize(),
				"log":     searchlog.Serialize(),
			})
		}

	})

	log.Fatal(app.Listen(":6767"))
}
