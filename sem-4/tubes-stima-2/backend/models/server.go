package models

type Request struct {
	URL      string `json:"url" form:"url" query:"url" validate:"required,url" message:"URL is required and must be a valid URL"`
	Type     string `json:"type" form:"type" query:"type" validate:"required,oneof=DFS BFS" message:"Type is required and must be either 'DFS' or 'BFS'"`
	Amount   int    `json:"amount" form:"amount" query:"amount" validate:"gte=0" message:"Amount must be a non-negative integer"`
	Selector string `json:"selector" form:"selector" query:"selector" validate:"required" message:"Selector is required"`
}
